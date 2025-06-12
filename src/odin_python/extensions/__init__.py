from .io_extension import MappedNumberIOExtension, ReferenceIOExtension
from .validation_extension import LimitValidationExtension
from .string_codec_extension import ReferenceStringCodecExtension
from typing import Annotated, Union
from pydantic import Discriminator

Extensions = Annotated[
    Union[MappedNumberIOExtension, LimitValidationExtension, ReferenceStringCodecExtension, ReferenceIOExtension],
    Discriminator("type"),
]
