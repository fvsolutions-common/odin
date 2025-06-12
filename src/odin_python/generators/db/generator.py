import hashlib
import time
from io import StringIO

from odin_db import OdinDBModel, OdinDBTypeDefinitionModel
from pydantic import BaseModel

from odin_python.data_types.type_registry import CustomDataType

from ..abstract_generator import AbstractGenerator, ModelContext
from .convertors import parameter_group_to_db, type_to_odin_db


class ODIN_DB_generator(AbstractGenerator):
    class Config(BaseModel):
        desctription: str = "Generic description"
        name: str = "ODIN"

        indent: int | None = 4

    config: Config

    def __init__(self, config: Config):
        super().__init__(config)
        self.config = config

    def generate(self, model_context: ModelContext, output_path: str | StringIO):
        db_params = parameter_group_to_db(model_context.root_model)
        hash = hashlib.md5()
        hash.update(db_params.model_dump_json().encode("utf-8"))

        db_types: dict[str, OdinDBTypeDefinitionModel] = {}

        for name, defined_type in model_context.types:
            if not isinstance(defined_type, CustomDataType):
                continue

            type = type_to_odin_db(defined_type.model)

            if type is None:
                continue

            db_types[name] = type

        db_model = OdinDBModel(
            name=self.config.name,
            description=self.config.desctription,
            creation_timestamp=time.time(),
            configuration_hash=int(hash.hexdigest(), 16),
            root=db_params,
            types=db_types,
        )

        self.save_to_file(output_path, db_model.model_dump_json(indent=self.config.indent))
