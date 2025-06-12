"""Contains resolve types derived from the type definition"""

import struct
from abc import ABC
from typing import Any, Dict, List

# import csnake as cc
from pydantic import BaseModel, Field

from odin_python.data_types.type_definition import (
    ExpandedUserTypeModel,
    ModelDataTypeDefintion,
    UserTypeModel,
)


class DataType(BaseModel, ABC):
    typename: str
    c_typename: str = Field(description="The C typename that corresponds to the data type")
    py_typename: str = Field(description="The Python typename that corresponds to the data type")
    size: int = Field(description="Size of the data type in bytes")
    default: Any = Field(description="The default value for the data type")
    struct_format: str = Field(description="The python struct format string for the data type")

    def extra_extension(self) -> str | None:
        return None

    @property
    def python_primitive_typename(self) -> str:
        if isinstance(self, BuiltinDataType):
            return self.primtive
        else:
            return self.py_typename


class BuiltinDataType(DataType):
    primtive: str = Field(description="The python primitive type for the data type")

    def to_python_struct(self) -> str:
        """Convert the data type to a python struct format string"""

        assert self.struct_format is not None, f"Python struct format string not defined for {self.typename}"

        return self.struct_format


class BooleanCDataType(BuiltinDataType):
    pass


class CharCDataType(BuiltinDataType):
    pass


class DataTypeModelDefinition(BaseModel):
    root: (
        DataType
        | dict[
            str,
            "DataTypeModelDefinition",
        ]
    )
    is_referenced: bool = Field(description="Whether the model is referenced based on another model")
    is_custom: bool = Field(description="Whether the model is a custom type")
    elements: int = Field(
        description="The amount of elements in this model, used for arrays",
    )
    type_default: Any = Field(description="The default value for the data type")

    @property
    def size(self) -> int:
        return struct.calcsize(self.struct_format(depth=0))

    @property
    def default(self) -> Any:

        if isinstance(self.root, DataType):
            if self.type_default is not None:
                return self.type_default
            elif self.elements == 1:
                return self.root.default
            else:
                return [self.root.default] * self.elements

        if isinstance(self.root, dict):
            defaults = {}
            for key, value in self.root.items():
                defaults[key] = value.default

            return defaults

        else:
            raise ValueError(f"Unknown type {self.root} for {self}")

    def struct_format(self, depth: int) -> str:
        if isinstance(self.root, DataType):
            return self.root.struct_format

        elif isinstance(self.root, dict):
            format = ""
            for key, value in self.root.items():
                # if isinstance(value, DataType):
                # raise ValueError(f"Presumed unused type {value} for {key}")
                # format += f"{value.elements}{value.struct_format}"
                if isinstance(value, DataTypeModelDefinition):
                    if value.is_referenced and depth < 1 and value.is_custom:
                        format += f"{value.size * value.elements}s"
                    else:
                        format += f"{value.elements}{value.struct_format(depth=depth - 1)}"
                else:
                    raise ValueError(f"Unknown type {value} for {key}")
            return format

        else:
            raise ValueError(f"Unknown type {self.root} for {self}")

    def to_flat_dict(self, depth: int) -> "dict[str, DataTypeModelDefinition]":
        """Convert the model to a flat list of type, will  flatten the model"""

        assert isinstance(self.root, dict)

        result: dict[str, DataTypeModelDefinition] = {}

        for key, value in self.root.items():
            if isinstance(value, DataTypeModelDefinition):
                result[key] = value  # type: ignore
            else:
                raise ValueError(f"Unknown type {value} for {key}")

        return result


class CustomDataType(DataType):
    string_serialiser: str | None = Field(description="The string serialiser for the data type")
    model: DataTypeModelDefinition = Field(
        description="The model for the data type",
    )

    def extra_extension(self) -> str | None:
        return None


BASE_DATA_TYPES: List[DataType] = [
    BuiltinDataType(typename="u64", size=8, c_typename="uint64_t", struct_format="Q", py_typename="OdinU64", default=0, primtive="int"),
    BuiltinDataType(typename="u32", size=4, c_typename="uint32_t", struct_format="I", py_typename="OdinU32", default=0, primtive="int"),
    BuiltinDataType(typename="u16", size=2, c_typename="uint16_t", struct_format="H", py_typename="OdinU16", default=0, primtive="int"),
    BuiltinDataType(typename="u8", size=1, c_typename="uint8_t", struct_format="B", py_typename="OdinU8", default=0, primtive="int"),
    BuiltinDataType(typename="i64", size=8, c_typename="int64_t", struct_format="Q", py_typename="OdinI64", default=0, primtive="int"),
    BuiltinDataType(typename="i32", size=4, c_typename="int32_t", struct_format="I", py_typename="OdinI32", default=0, primtive="int"),
    BuiltinDataType(typename="i16", size=2, c_typename="int16_t", struct_format="H", py_typename="OdinI16", default=0, primtive="int"),
    BuiltinDataType(typename="i8", size=1, c_typename="int8_t", struct_format="B", py_typename="OdinI8", default=0, primtive="int"),
    BuiltinDataType(typename="f32", size=4, c_typename="float", struct_format="f", py_typename="OdinF32", default=0.0, primtive="float"),
    BuiltinDataType(typename="f64", size=8, c_typename="double", struct_format="d", py_typename="OdinF64", default=0.0, primtive="float"),
    BooleanCDataType(typename="bool", size=1, c_typename="bool", struct_format="?", py_typename="OdinBool", default=False, primtive="int"),
    CharCDataType(typename="char", size=1, c_typename="char", struct_format="B", py_typename="OdinChar", default=0, primtive="int"),
]


def resolve_datatype_model(
    type_registry: "TypeRegistry",
    type: UserTypeModel | ExpandedUserTypeModel | Dict[str, UserTypeModel | ExpandedUserTypeModel],
) -> DataTypeModelDefinition:
    """Convert a data type model to a data type"""

    if isinstance(type, dict):
        models = {}
        for type_value, value in type.items():
            models[type_value] = resolve_datatype_model(type_registry, value.expand)

        return DataTypeModelDefinition(root=models, is_referenced=False, is_custom=False, elements=1, type_default=None)

    elif isinstance(type, UserTypeModel) | isinstance(type, ExpandedUserTypeModel):
        datatype = type_registry.find_type(type.type)
        return DataTypeModelDefinition(
            root=datatype,  # type: ignore
            is_referenced=True,
            is_custom=isinstance(datatype, CustomDataType),
            elements=type.elements if type.elements is not None else 1,
            type_default=type.default,
        )

    else:
        raise ValueError(f"Unknown type {type}")


class TypeRegistry:
    """The type registry, us used to store datatypes, it allows adding custom types"""

    def __init__(self):
        self.types: Dict[str, DataType] = {}

    def register_custom_datatype(self, name: str, type: ModelDataTypeDefintion):
        """Register a custom data type"""

        if name in self.types:
            raise ValueError(f"Type {name} already registered")

        model = resolve_datatype_model(self, type.model)

        # Create the custom data type
        custom_type = CustomDataType(
            typename=name,
            c_typename=type.c_typename if type.c_typename is not None else f"{name}_t",
            py_typename=type.py_typename if type.py_typename is not None else f"Odin{name.title().replace('_', '')}",
            size=model.size,
            default=model.default,
            struct_format=model.struct_format(depth=0),
            model=model,
            string_serialiser=type.string_serialiser,
        )

        # Register the custom data type
        self.register(custom_type)

    def register(self, type: DataType | List[DataType]):
        """Register a new type"""

        if isinstance(type, list):
            for t in type:
                self.register(t)
            return

        if type.typename in self.types:
            raise ValueError(f"Type {type.typename} already registered")

        self.types[type.typename] = type

    def find_type(self, name: str) -> DataType:
        """Get a type by name"""

        if name not in self.types:
            raise ValueError(f"Type {name} not found")

        return self.types[name]

    def __repr__(self):
        # Return a list of all type names
        return f"TypeRegistry({', '.join(self.types.keys())})"

    def __getitem__(self, item: str):
        """Get a type by name"""
        return self.find_type(item)

    def __iter__(self):
        """Iterate over the types in the registry"""
        return iter(self.types.items())
