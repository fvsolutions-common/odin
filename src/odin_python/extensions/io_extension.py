from typing import TYPE_CHECKING, Literal

from pydantic import Field

from ..generators.c.model import ODIN_ExtesionModel
from ..generators.c.odin_defs import ODIN_ExtensionEnum

from .common import BaseExtension

if TYPE_CHECKING:
    from ..parameter.parameter import BaseParameterModel


class IOExtension(BaseExtension):
    pass


class MappedNumberIOExtension(IOExtension):
    type: Literal["io_mapped_numner"]
    reference: str = Field(description="Reference to another odin parameter")
    scale: float = Field(description="Scale factor", default=1.0)
    offset: float = Field(description="Offset", default=0.0)

    def as_literal(self, parameter: "BaseParameterModel", next: str | None) -> str:
        # Get root parameter
        referenced_parameter = parameter.root.find_parameter_by_object_name(self.reference)
        assert referenced_parameter, f"Could not find parameter {self.reference}"

        # TODO check if variables are set correctly

        model = ODIN_ExtesionModel(
            type=ODIN_ExtensionEnum.ODIN_EXTENSION_TYPE_IO,
            ops="ODIN_extension_io_mapped_number_ops",
            parameters=f"(mapped_number_parameters_t){{ .reference = &{referenced_parameter.absolute_object_reference}, .scale = {self.scale}, .offset = {self.offset} }}",
            next=next if next else None,
        )

        return model.variable


class ReferenceIOExtension(BaseExtension):
    type: Literal["custom_io"]
    reference: str = Field(description="Reference to the C extension")

    def as_literal(self, parameter: "BaseParameterModel", next: str | None) -> str:
        # Get root parameter
        # referenced_parameter = parameter.root.find_parameter_by_object_name(self.reference)
        # assert referenced_parameter, f"Could not find parameter {self.reference}"

        # TODO check if variables are set correctly

        model = ODIN_ExtesionModel(
            type=ODIN_ExtensionEnum.ODIN_EXTENSION_TYPE_IO,
            ops=self.reference,
            parameters=None,
            next=next if next else None,
        )

        return model.variable
