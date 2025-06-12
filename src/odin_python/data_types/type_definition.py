"""Data type definition for the yaml definition."""

from typing import Any, Dict


from pydantic import BaseModel, ConfigDict, Field, RootModel

class ExpandedUserTypeModel(BaseModel):
    model_config = ConfigDict(
        extra="forbid",
    )

    type: "str"
    default: "list[Any] | Any|None" = Field(description="The default value for the data type", default=None)
    elements: int | None = Field(
        default=None,
        description="The size of the data type in bytes, if not specified the size will be calculated from the type",
    )

    @property
    def expand(self) -> "ExpandedUserTypeModel":
        return self


class UserTypeModel(RootModel[str]):
    @property
    def expand(self) -> ExpandedUserTypeModel:
        return ExpandedUserTypeModel(
            type=self.root,
            default=None,
            elements=None,
        )

    @property
    def type(self) -> str:
        return self.root

    @property
    def default(self) -> None:
        return None

    @property
    def elements(self) -> None:
        return None


class DataTypeDefintion(BaseModel):
    model_config = ConfigDict(
        extra="forbid",
    )


    description: str = Field(description="Description of the data type", default="Not specified")
    string_serialiser: str | None = Field(default=None, description="The string serialiser extension for the data type")

    c_typename: str | None = Field(
        default=None,
        description="The C typename that corresponds to the data type, if not specified _t will be added to the name",
    )
    py_typename: str | None = Field(
        default=None,
        description="The Python typename that corresponds to the data type ifnot specified the type will be camelcased with a Odin prefix",
    )


class ModelDataTypeDefintion(DataTypeDefintion):
    """Dictionary of data types with the name as the key and the data type as the value
    This represents a struct
    """

    model: Dict[str, UserTypeModel | ExpandedUserTypeModel] = Field(
        description="The model for the data type",
    )
