from typing import Literal

from pydantic import Field

from ..generators.c.model import ODIN_ExtesionModel
from ..generators.c.odin_defs import ODIN_ExtensionEnum
from .common import BaseExtension
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from ..parameter.parameter import BaseParameterModel


class StringCodecExtension(BaseExtension): ...


class ReferenceStringCodecExtension(StringCodecExtension):
    type: Literal["string_codec_reference"]
    reference: str = Field(description="Refercene to the codec")

    def as_literal(self, parameter: "BaseParameterModel", next: str | None) -> str:
        model = ODIN_ExtesionModel(
            type=ODIN_ExtensionEnum.ODIN_EXTENSION_TYPE_STRING_CODEC,
            ops=self.reference,
            parameters=None,
            next=next if next else None,
        )
        return model.variable

    # next
