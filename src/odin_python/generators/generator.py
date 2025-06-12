from enum import Enum
from pydantic import ConfigDict, Field
from .abstract_generator import ModelContext
from .c.generator import CGenerator
from .db.generator import ODIN_DB_generator
from .py.generator import PYGenerator
from .pdf.generator import DocGenerator
from .abstract_generator import BaseModel
import os


class GeneratorConfigurations(BaseModel):
    model_config = ConfigDict(
        extra="forbid",
    )

    c_generator: CGenerator.Config = Field(
        default_factory=CGenerator.Config,
        description="C generator configuration",
    )
    python_generator: PYGenerator.Config = Field(
        default_factory=PYGenerator.Config,
        description="Python generator configuration",
    )
    doc_generator: DocGenerator.Config = Field(
        default_factory=DocGenerator.Config,
        description="Doc generator configuration",
    )
    db_generator: ODIN_DB_generator.Config = Field(
        default_factory=ODIN_DB_generator.Config,
        description="DB generator configuration",
    )


class GeneratorTarget(Enum):
    DOC = "doc"
    DB = "db"
    PY = "py"
    C = "c"

    @staticmethod
    def all() -> "list[GeneratorTarget]":
        results = []
        for target in GeneratorTarget:
            results.append(target)
        return results

    @classmethod
    def from_string(cls, name: str) -> "GeneratorTarget":
        for target in cls.all():
            if target.name == name:
                return target
        raise ValueError(f"Unknown generator target: {name}")


def generator(
    name: str,
    model_context: ModelContext,
    output_dir: str,
    target: GeneratorTarget,
    generator_config: GeneratorConfigurations,
):
    # Generate C code
    if target == GeneratorTarget.C:
        c_generator = CGenerator(generator_config.c_generator)
        c_generator.generate(
            model_context=model_context,
            output_path=os.path.join(output_dir, f"{name}.c"),
            type="source",
        )

        c_generator.generate(
            model_context=model_context,
            output_path=os.path.join(output_dir, f"{name}.h"),
            type="header",
        )

    elif target == GeneratorTarget.PY:
        py_generator = PYGenerator(generator_config.python_generator)
        py_generator.generate(
            model_context=model_context,
            output_path=os.path.join(output_dir, f"{name}"),
        )

    elif target == GeneratorTarget.DB:
        db_generator = ODIN_DB_generator(generator_config.db_generator)
        db_generator.generate(
            model_context=model_context,
            output_path=os.path.join(output_dir, f"{name}.odin"),
        )

    elif target == GeneratorTarget.DOC:
        doc_generator = DocGenerator(generator_config.doc_generator)
        doc_generator.generate(
            model_context=model_context,
            output_path=os.path.join(output_dir, f"{name}.pdf"),
        )
    else:
        raise ValueError(f"Unknown generator target: {target}")
