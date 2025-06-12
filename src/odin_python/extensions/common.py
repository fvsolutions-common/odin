from abc import ABC, abstractmethod

from pydantic import BaseModel
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from ..parameter.parameter import BaseParameterModel


class BaseExtension(BaseModel, ABC):
    @abstractmethod
    def as_literal(self, parameter: "BaseParameterModel", next: str | None) -> str: ...
