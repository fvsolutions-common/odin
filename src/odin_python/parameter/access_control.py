from enum import Enum, auto
from typing import Dict, List, TYPE_CHECKING, Set


from pydantic import (
    BaseModel,
    Field,
    PrivateAttr,
    RootModel,
    model_validator,
)

MAX_ACCESS_GROUPS = 6  # Max supported unique access groups
GROUP_ACCESS_PREFIX = "ODIN_ACCESS_"
ACTION_PREFIX = "ODIN_ACCESS_"

if TYPE_CHECKING:
    from .parameter import BaseParameterModel  # pragma: no cover


class AccessControlEnum(Enum):
    """Access control enum, list all the possible access control items
    Needs to match the odin definitions in the c library
    """

    READ = auto()
    WRITE = auto()
    LOG_WRITE = auto()
    LOG_READ = auto()

    @staticmethod
    def from_compact_representation(string: str) -> Set["AccessControlEnum"]:
        """Converts compact representation to set of access control enum"""
        items = set()
        if "R" in string:
            items.add(AccessControlEnum.READ)
            # Remove R from the string
            string = string.replace("R", "")

        if "W" in string:
            items.add(AccessControlEnum.WRITE)
            string = string.replace("W", "")

        # If there are any other characters left, raise an error
        if len(string) > 0:
            raise ValueError(f"Unknown access control item {string}")

        return items

    @classmethod
    def from_string(cls, input: str) -> "AccessControlEnum":
        """Converts string to access control enum"""
        if input.upper() in AccessControlEnum.__members__:
            return AccessControlEnum[input.upper()]

        elif input == "R":
            return AccessControlEnum.READ
        elif input == "W":
            return AccessControlEnum.WRITE

        else:
            raise ValueError(f"Unknown access control item '{input}' is invalid")

    @staticmethod
    def normalize(
        input: "str | Set[str] | Set[AccessControlEnum]",
    ) -> "Set[AccessControlEnum]":
        """Parses input to a set of access control enums"""
        if isinstance(input, str):
            return AccessControlEnum.from_compact_representation(input)

        elif isinstance(input, set):
            items = set()
            for item in input:
                if isinstance(item, str):
                    items.add(AccessControlEnum.from_string(item))
                else:
                    items.add(item)
            return items


class AccesControlDefinition(BaseModel):
    """Access control definition for a single access group"""

    override: Set[str] | Set[AccessControlEnum] | str | None = Field(
        default=None,
        description="Override the default access control for the given group, if not specified it will not override",
    )

    default: Set[str] | Set[AccessControlEnum] | str = Field(default_factory=set, description="Default access control for the given group")

    @model_validator(mode="after")  # type: ignore
    def convert_fields(cls, model: "AccesControlDefinition") -> "AccesControlDefinition":
        """Parses the fields to the correct type"""

        model.default = AccessControlEnum.normalize(model.default)

        if model.override is not None:
            model.override = AccessControlEnum.normalize(model.override)
            model.default = model.default.union(model.override)

        return model

    def merge(self, other: "AccesControlDefinition") -> "AccesControlDefinition":
        """Merges two access control definitions

        Args:
            other (AccesControlDefinition): Other access control definition to merge with
        """

        # If the child overrides the parent, use the child
        if other.override is not None:
            return AccesControlDefinition(
                override=other.override,
                default=other.override,  # type: ignore
            )

        # Otherwise merge the default permissions, and copy the parent override
        else:
            return AccesControlDefinition(
                override=self.override,
                default=self.default.union(other.default),  # type: ignore
            )  # type: ignore

    @classmethod
    def normalize(cls, input: "str | Set[str]|AccesControlDefinition") -> "AccesControlDefinition":
        """Normalizes input to an access control definition"""
        if isinstance(input, AccesControlDefinition):
            return input

        return cls(default=AccessControlEnum.normalize(input))

    def to_c_definition(self, group_name: str) -> str | None:
        """Generates the C definition for the access control group"""

        permissions = []

        for permission in self.default:
            assert isinstance(permission, AccessControlEnum), "Invalid permission type"
            permissions.append(f"{ACTION_PREFIX}{permission.name.upper()}")

        if len(permissions) == 0:
            return None

        return f"({GROUP_ACCESS_PREFIX}{group_name.upper()} & ({' | '.join(permissions)}))"


class AccessControlCollection(RootModel[Dict[str, AccesControlDefinition | Set[str] | str]]):
    # Reference to parent parameter model
    _parent: "BaseParameterModel | None" = PrivateAttr(default=None)
    _access_groups: Dict[int, str] | None = PrivateAttr(default=None)

    pass

    @model_validator(mode="after")  # type: ignore
    def validate_access_control(cls, model: "AccessControlCollection") -> "AccessControlCollection":
        """Validates the access control collection"""

        return model.normalize()

    @classmethod
    def empty(cls):
        return cls({})

    def to_c_definition(self) -> str:
        """Generates the C definition for the access control collection"""

        definitions: List[str] = []

        for group_name, entry in self.root.items():
            assert isinstance(entry, AccesControlDefinition), "Invalid entry type"

            definition = entry.to_c_definition(group_name)
            if definition is not None:
                definitions.append(definition)

        # If no definitions are found, return 0, to avoid a c syntax error
        if len(definitions) == 0:
            return "0"

        # Otherwise merge all the definitions into a single string
        return " | ".join(definitions)

    def normalize(self) -> "AccessControlCollection":
        """Normalizes the access control collection"""

        for name, item in self.root.items():
            if not isinstance(item, AccesControlDefinition):
                item = AccesControlDefinition.normalize(item)

            self.root[name] = item

        return self

    def collapse(self) -> "AccessControlCollection":
        """Collapses the access control collection into a single access control collection"""

        # Create a list of all the access control collections from the root to the current collection
        collections = self.recursive_get_build_permission_chain()

        # Start with itself
        merged_collection = self

        # And merge all the children
        for collection in collections:
            merged_collection = merged_collection.merge(collection)

        return merged_collection

    def recursive_get_build_permission_chain(self) -> "List[AccessControlCollection]":
        """Recursively gets the build permission chain"""

        # I dont like this import here, but it is necessary to avoid circular imports
        # There probably are better ways to do this, but i did not bother to implement them
        # its only imported once anyway

        from odin_python.parameter import RootParameterModel

        self_parameter = self._parent

        access_control: List[AccessControlCollection]

        if isinstance(self_parameter, RootParameterModel):
            access_control = []

        else:
            assert self_parameter is not None, "No parameter found for this entry"
            assert self_parameter._parent is not None, "No parent found for the parameter"

            access_control = AccessControlCollection.recursive_get_build_permission_chain(self_parameter._parent.access_control)

        access_control.append(self)
        return access_control

    def get_access_group_ids(self) -> Dict[int, str]:
        """Calculates the access group ids for the access control collection"""

        if self._access_groups is None:
            self._access_groups = {}

            for id, (group_name, entry) in enumerate(self.root.items()):
                self._access_groups[id] = group_name

                if id >= MAX_ACCESS_GROUPS:
                    raise ValueError(f"Only {MAX_ACCESS_GROUPS} access groups are allowed")

        return self._access_groups

    def merge(self, other: "AccessControlCollection") -> "AccessControlCollection":
        """Merges two access control collection together, inheriting or overriding the permissions"""

        items: Dict[str, AccesControlDefinition] = {}

        for name, item in self.root.items():
            assert isinstance(item, AccesControlDefinition), "Invalid item type"
            items[name] = item

        for name, item in other.root.items():
            assert isinstance(item, AccesControlDefinition), "Invalid item type"

            if name not in items:
                items[name] = item
            else:
                items[name] = items[name].merge(item)

        return AccessControlCollection(root=items)  # type: ignore
