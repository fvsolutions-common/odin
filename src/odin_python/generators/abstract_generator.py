from abc import ABC, abstractmethod
from io import StringIO

from pydantic import BaseModel

from odin_python.parameter.parameter import ParameterGroupModel

from ..data_types.type_registry import TypeRegistry
from ..parameter import RootParameterModel
from dataclasses import dataclass


@dataclass
class ModelContext:
    types: TypeRegistry
    root_model: RootParameterModel
    collections: dict[str, ParameterGroupModel] | None = None


class AbstractGenerator(ABC):
    def __init__(self, config: BaseModel):
        self.config = config

    @abstractmethod
    def generate(self, model_context: ModelContext, output_path: str | StringIO) -> None: ...

    def save_to_file(self, output_path: str | StringIO, data: str) -> None:
        if isinstance(output_path, StringIO):
            output_path.write(data)
        else:
            with open(output_path, "w") as f:
                f.write(data)
