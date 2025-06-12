from typing import TYPE_CHECKING, Literal

from pydantic import Field

from ..generators.c.odin_defs import ODIN_ExtensionEnum
from ..generators.c.model import ODIN_ExtesionModel
from .common import BaseExtension

if TYPE_CHECKING:
    from ..parameter.parameter import BaseParameterModel


class ValidationExtension(BaseExtension): ...


class LimitValidationExtension(ValidationExtension):
    type: Literal["validation_limit_value"]
    min: float | None = Field(default=None, title="Minimum value, if not set, no minimum value is enforced")
    max: float | None = Field(
        default=None,
        title="Maximum allowed value, if not set, no maximum value is enforced",
    )

    def as_literal(self, parameter: "BaseParameterModel", next: str | None) -> str:
        # TODO check if paramter is a regular parameter that can be cast to float

        if self.min is None:
            self.min = -float("inf")
        if self.max is None:
            self.max = float("inf")

        model = ODIN_ExtesionModel(
            type=ODIN_ExtensionEnum.ODIN_EXTENSION_TYPE_VALIDATE,
            ops="ODIN_validate_extension_ops",
            parameters=f"(range_parameter_t){{ .max = {self.max}, .min = {self.min} }}",
            next=next if next else None,
        )

        # value = f"(ODIN_extension_t[]){{ {{.type=ODIN_EXTENSION_TYPE_IO, .ops =  &ODIN_validate_extension_ops, .data = &(range_parameter_t){{ .max = {self.max}, .min = {self.min} }} }} }}"
        return model.variable

    # next
