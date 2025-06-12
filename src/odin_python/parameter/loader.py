from typing import Dict, Literal

from pydantic import BaseModel, ConfigDict, Field, RootModel, ValidationError
from pydantic_yaml import parse_yaml_file_as

from odin_python.data_types.type_definition import ModelDataTypeDefintion
from odin_python.generators.generator import GeneratorConfigurations

# from ..data_types.input_type_model import TypeSpecifciationCollectionModel
from ..data_types.type_registry import BASE_DATA_TYPES, TypeRegistry
from ..generators.abstract_generator import ModelContext
from .parameter import (
    AccessControlCollection,
    ArrayParameterModel,
    CollectionModel,
    ParameterGroupModel,
    ParameterModel,
    RootParameterModel,
    VectorParameterModel,
)


class CollectionDescriptionModel(BaseModel):
    description: str = Field(
        default="",
        description="Description of the collection",
    )

    children: list[str] = Field(
        default_factory=list,
        description="List of parameters in the collection",
    )

    def to_group_model(
        self,
        group_name: str,
        root_parameters: RootParameterModel,
    ) -> CollectionModel:
        group_model = CollectionModel(
            type="collection",
            description=self.description,
            children={},
        )

        group_model._name = group_name

        for param_name in self.children:
            found_params = root_parameters.find_parameters_by_object_name(param_name)

            if len(found_params) == 0:
                raise ValueError(f"Parameter {param_name} not found in group {group_name}")

            for found_param in found_params:
                assert isinstance(
                    found_param,
                    ParameterModel | ArrayParameterModel | VectorParameterModel | ParameterGroupModel,
                ), "Parameter not found"

                group_model.children[found_param.global_name] = found_param

        return group_model


class AdvancedLoaderModel(BaseModel):
    model_config = ConfigDict(
        extra="forbid",
    )
    config: GeneratorConfigurations = Field(
        default_factory=GeneratorConfigurations,
        description="Generator configurations",
    )

    access_control: AccessControlCollection = Field(
        default_factory=AccessControlCollection.empty,
        description="Access control for the parameters",
    )

    types: dict[str, ModelDataTypeDefintion] = Field(
        default_factory=dict,
        description="Type definitions",
    )

    collections: Dict[str, CollectionDescriptionModel] = Field(
        default_factory=dict,
        description="Collections of parameters",
    )
    parameters: Dict[
        str,
        ParameterModel | ParameterGroupModel | ArrayParameterModel | VectorParameterModel,
    ]
    id_space_shift: int = Field(description="ID space shift for the parameters")

    @classmethod
    def from_yaml(cls, file_path: str):
        try:
            return parse_yaml_file_as(cls, file_path)
        except ValidationError as validation_error:
            print(validation_error.errors())
            for error in validation_error.errors():
                print(error["loc"])
                print(error["msg"])

            raise Exception("Failed to parse yaml file")


class ConfigurationReader:
    def __init__(self):
        pass

    def load(self, file_path: str, type: Literal["advanced"]) -> tuple[ModelContext, GeneratorConfigurations]:
        registry = TypeRegistry()
        registry.register(BASE_DATA_TYPES)

        try:
            if type == "advanced":
                model = AdvancedLoaderModel.from_yaml(file_path)

                for type_name, type_definition in model.types.items():
                    registry.register_custom_datatype(type_name, type_definition)

                root_parameters = RootParameterModel(
                    children=model.parameters,
                    access_control=model.access_control,
                    id_space_shift=model.id_space_shift,
                )

            else:
                raise ValueError("Invalid type. Use 'advanced'.")

            root_parameters.post_load_resolve(None, "root", registry)

            collections = {}
            for group_name, group in model.collections.items():
                collections[group_name] = group.to_group_model(group_name, root_parameters)

            return ModelContext(
                root_model=root_parameters,
                types=registry,
                collections=collections,
            ), model.config

        except ValidationError as validation_error:
            print(validation_error.errors())
            for error in validation_error.errors():
                print(error["loc"])
                print(error["msg"])

            raise Exception("Failed to parse yaml file")
        