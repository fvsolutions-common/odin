from abc import ABC
from dataclasses import dataclass
from typing import Annotated, Any, Dict, List, Literal

from pydantic import BaseModel, ConfigDict, Discriminator, Field, PrivateAttr

from ..data_types.type_registry import (
    CustomDataType,
    DataType,
    TypeRegistry,
)
from ..extensions import Extensions
from ..extensions.string_codec_extension import ReferenceStringCodecExtension
from ..parameter.access_control import AccessControlCollection

MAX_ID_SIZE = 32


@dataclass
class C_Names:
    objects_type: str
    objects_name: str
    variables_type: str
    variables_name: str
    groups_name: str


class BaseParameterModel(BaseModel, ABC):
    """The base parameter model, is an abstract class that all parameter models inherit from

    It contains the basic properties and methods that all parameter models should have
    """

    model_config = ConfigDict(extra="forbid")

    # Name of the parameter, set by the post_load_resolve method
    _name: str = PrivateAttr()

    # Reference to the parent of the parameter, set by the post_load_resolve method
    _parent: "BaseParameterModel|None" = PrivateAttr(default=None)

    local_id: int = Field(description="Local id of the parameter, unique within the parent")

    description: str | None = Field(description="Description of the parameter, optional", default=None)

    access_control: AccessControlCollection = Field(
        description="Access control configuration for the parameter",
        default_factory=AccessControlCollection.empty,
    )

    # Reference to the variable which stores the data
    _absolute_variable_reference: str | None = PrivateAttr(default=None)

    # Reference to the odin parameter, which represents the parameter in the odin runtime
    _absolute_object_reference: str | None = PrivateAttr(default=None)

    def post_load_resolve(
        self,
        parent: "BaseParameterModel | None",
        name: str,
        type_registry: TypeRegistry,
    ):
        """Post load resolve method, must to be called after the model is created"""

        del type_registry  # type_registry is not used here
        self._parent = parent
        self._name = name
        self.access_control._parent = self

    def initialise_types(self):
        assert self._parent is not None, "Parent is not set"

        self._absolute_variable_reference = f"{self._parent._absolute_variable_reference}.{self._name}"
        self._absolute_object_reference = f"{self._parent._absolute_object_reference}.{self._name}"

        if isinstance(self, BaseParameterGroupModel):
            self._absolute_group_reference = f"{self._parent._absolute_group_reference}_{self._name}"

            # If the parent is a group, then the variable reference is the parent name
            for child in self.children.values():
                child.initialise_types()

    @property
    def absolute_variable_reference(self) -> str:
        assert self._absolute_variable_reference is not None, "Variable reference is not set"
        return self._absolute_variable_reference

    @property
    def absolute_object_reference(self) -> str:
        assert self._absolute_object_reference is not None, "Object reference is not set"
        return self._absolute_object_reference

    @property
    def resolved_description(self) -> str:
        """Returns the description of the parameter, if not set returns a default value"""
        return self.description if self.description else "No description"

    @property
    def global_id(self) -> int:
        """Calculates the global id of the parameter, by going up the tree"""

        if self._parent is None:
            return self.local_id

        if isinstance(self._parent, RootParameterModel):
            return self.local_id << (MAX_ID_SIZE - self.global_shift)

        elif isinstance(self._parent, ParameterGroupModel):
            return self._parent.global_id | (self.local_id << (MAX_ID_SIZE - self.global_shift))
        else:
            raise ValueError("Parent is not a ParameterGroupModel")

    @property
    def global_name(self) -> str:
        """Calculates the global name of the parameter, by going up the tree"""

        if self._parent is None:
            return self._name

        return f"{self._parent.global_name}.{self._name}"

    @property
    def global_shift(self) -> int:
        """Adds the shift of all the parents to upstream"""

        if self._parent is None:
            return 0

        if isinstance(self._parent, RootParameterModel):
            return self._parent.id_space_shift

        elif isinstance(self._parent, ParameterGroupModel):
            return self._parent.global_shift + self._parent.id_space_shift

        else:
            raise ValueError("Parent is not a ParameterGroupModel")

    @property
    def root(self) -> "RootParameterModel":
        """Returns the root of the tree"""

        if self._parent is None:
            assert isinstance(self, RootParameterModel), "Root is not set"
            return self

        if isinstance(self._parent, RootParameterModel):
            return self._parent

        return self._parent.root


class BaseBaseParameterModel(BaseParameterModel):
    primitive: str = Field(description="Primitive type of the parameter")

    # The type this parameter resolves to, set by the post_load_resolve method
    _resolved_type: DataType = PrivateAttr()

    extensions: List[Extensions] = Field(description="Extensions of the parameter", default_factory=list)

    reference: str | None = Field(
        description="Sets the storage location of the parameter, if not set it will be managed internally",
        default=None,
    )


class ParameterModel(BaseBaseParameterModel):
    """Parameter model, represents a single parameter"""

    type: Literal["parameter"] | Literal["void"]

    # Default value of the parameter, it not set it will use the default value of the primitive type
    default: Any | None = Field(description="Default value of the parameter", default=None)

    def post_load_resolve(self, parent: BaseParameterModel | None, name: str, type_registry: TypeRegistry):
        super().post_load_resolve(parent, name, type_registry)
        self._resolved_type = type_registry.find_type(self.primitive)

        if isinstance(self._resolved_type, CustomDataType) and self._resolved_type.string_serialiser:
            # Add the string codec extension to the parameter
            if self._resolved_type.string_serialiser:
                self.extensions.append(
                    ReferenceStringCodecExtension(
                        type="string_codec_reference",
                        reference=self._resolved_type.string_serialiser,
                    )
                )

    def one_line_summary(self, level=0):
        return f"{level * '  '}{self._name} (0x{self.global_id:08X}): {self.primitive} - {self._resolved_type.size} bytes"


class ArrayParameterModel(BaseBaseParameterModel):
    type: Literal["array"]
    default: List[Any] | str | None = Field(description="Default value of the parameter", default=None)
    elements: int = Field(description="Number of elements in the array")

    def post_load_resolve(self, parent: BaseParameterModel | None, name: str, type_registry: TypeRegistry):
        super().post_load_resolve(parent, name, type_registry)
        self._resolved_type = type_registry.find_type(self.primitive)

    def one_line_summary(self, level=0):
        return f"{level * '  '}{self._name} (0x{self.global_id:08X}): {self.primitive} - {self._resolved_type.size} bytes"


class VectorParameterModel(BaseBaseParameterModel):
    type: Literal["vector"]
    default: List[Any] | str | None = Field(description="Default value of the parameter", default=None)
    max_elements: int = Field(description="Number of elements in the array")

    def post_load_resolve(self, parent: BaseParameterModel | None, name: str, type_registry: TypeRegistry):
        super().post_load_resolve(parent, name, type_registry)
        self._resolved_type = type_registry.find_type(self.primitive)

    def one_line_summary(self, level=0):
        return f"{level * '  '}{self._name} (0x{self.global_id:08X}): {self.primitive} - {self._resolved_type.size} bytes"


class BaseParameterGroupModel(BaseParameterModel):
    children: """Dict[
        str,
        Annotated[
            ParameterModel
            |ArrayParameterModel
            |VectorParameterModel
            |ParameterGroupModel,
            Discriminator("type"),
        ]
    ]"""
    id_space_shift: int = Field(description="Number of bits to shift the children ids")

    # Reference to the variable which stores the group
    _absolute_group_reference: str | None = PrivateAttr(default=None)

    @property
    def absolute_group_reference(self) -> str:
        assert self._absolute_group_reference is not None, "Group reference is not set"
        return self._absolute_group_reference

    def post_load_resolve(
        self,
        parent: BaseParameterModel | None,
        name: str,
        type_registry: TypeRegistry,
    ):
        super().post_load_resolve(parent, name, type_registry)
        for child_name, child in self.children.items():
            child.post_load_resolve(self, child_name, type_registry)

        self.validate_local_id()

        # Validate ids
        self.build_global_id_map()

    def one_line_summary(self, level=0):
        val = f"{level * '  '}{self._name} (0x{self.global_id:08X})"
        for child in self.children.values():
            val += f"\n{child.one_line_summary(level + 1)}"
        return val

    def validate_local_id(self):
        local_id_map = {}
        for child in self.children.values():
            if child.local_id in local_id_map:
                raise ValueError(f"Id conflict found: {child.local_id} in {self._name}")
            else:
                local_id_map[child.local_id] = child

            # check if local id respects bit shift
            if child.local_id >= (1 << self.id_space_shift):
                raise ValueError(f"Local id {child.local_id} is larger than the id shift {self.id_space_shift} in {self._name}")
            
            if child.local_id < 0:
                raise ValueError(f"Local id {child.local_id} is negative in {self._name}")
            
            if isinstance(child, BaseParameterGroupModel):
                child.validate_local_id()

    def build_global_id_map(self) -> Dict[int, BaseParameterModel]:
        """Builds a map of the global ids to the parameters"""

        global_id_map: Dict[int, BaseParameterModel] = {self.global_id: self}

        for child in self.children.values():
            if isinstance(child, BaseParameterGroupModel):
                global_id_map.update(child.build_global_id_map())

            elif isinstance(child, BaseParameterModel):
                global_id_map[child.global_id] = child

        return global_id_map

    @property
    def child_map(self) -> Dict[str, BaseParameterModel]:
        child_map = {}
        for child in self.children.values():
            child_map[child._name] = child

        return child_map

    def find_parameters_by_object_name(self, name: str) -> list[BaseParameterModel]:
        # Based name
        split_name = name.split(".")
        base_name = split_name[0]

        child_map = self.child_map

        if base_name == "*":
            return list(child_map.values())

        if base_name not in child_map:
            return []

        parameter = child_map[base_name]

        if isinstance(parameter, BaseParameterGroupModel):
            if len(split_name) == 1:
                return [parameter]
            else:
                return parameter.find_parameters_by_object_name(".".join(split_name[1:]))

        return [parameter]

    def find_parameter_by_object_name(self, name: str) -> BaseParameterModel | None:
        parameters = self.find_parameters_by_object_name(name)
        if parameters is None or len(parameters) == 0:
            return None

        if len(parameters) > 1:
            raise ValueError(f"Multiple parameters found for {name}: {parameters}")

        return parameters[0]

    def to_flat_list(self) -> List[BaseParameterModel]:
        flat_list = []
        for child in self.children.values():
            flat_list.append(child)

            if isinstance(child, BaseParameterGroupModel):
                flat_list.extend(child.to_flat_list())

        return flat_list


class ParameterGroupModel(BaseParameterGroupModel):
    type: Literal["group"]
    pass


class CollectionModel(BaseParameterGroupModel):
    type: Literal["collection"]
    id_space_shift: int = 8
    local_id: int = 0


class RootParameterModel(BaseParameterGroupModel):
    local_id: int = 0

    _c_types: C_Names | None = PrivateAttr(default=None)

    def dump_json(self):
        return self.model_dump_json(indent=2)

    def to_access_control_header(self) -> List[str]:
        # todo possible move this into acces contol info?
        assert self.access_control is not None, "Access control info is not set"
        groups = self.access_control.get_access_group_ids()

        defines = []
        for id, name in groups.items():
            defines.append(f"#define ODIN_ACCESS_{name.upper()} ODIN_ACCESS_GROUP_{id}")

        return defines

    def initialise_types(self, types: C_Names):  # type: ignore
        self._c_types = types
        self._absolute_variable_reference = types.variables_name
        self._absolute_object_reference = types.objects_name
        self._absolute_group_reference = types.groups_name

        for child in self.children.values():
            child.initialise_types()
