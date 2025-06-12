import asyncio
from abc import ABC, abstractmethod
from typing import Any, Generic, Self, Type, TypeVar

from pydantic import BaseModel, ConfigDict

T = TypeVar("T")


class TemplateInterface:
    async def get_single(self, id: int) -> bytes: ...

    async def set_request(self, data: dict[int, bytes]): ...


class ConfiguredBaseModel(BaseModel):
    model_config = ConfigDict(
        ser_json_bytes="hex",
    )

class GenericModel(ABC):
    @abstractmethod
    def encode_to_bytes(self) -> bytes:
        pass

    @classmethod
    @abstractmethod
    def decode_from_bytes(cls, data: bytes) -> Self:
        pass


class ODINArrayEntry(Generic[T]):
    interface: TemplateInterface

    def __init__(self, id: int, cls: Type[GenericModel], element_size: int, elements: int, interface: TemplateInterface):
        self.id = id
        self.type_class = cls
        self.interface = interface
        self.element_size = element_size
        self.elements = elements

    def decode_from_bytes(self, data: bytes) -> list[T]:
        array = [self.type_class.decode_from_bytes(data[i : i + self.element_size]) for i in range(0, len(data), self.element_size)]  # type: ignore

        if len(array) != self.elements:
            raise ValueError(f"Value length {len(array)} does not match expected length {self.elements}")

        return array  # type: ignore

    async def read(self) -> list[T]:
        return self.decode_from_bytes(await self.interface.get_single(self.id))  # type: ignore

    async def write(self, value: list[T]):
        if len(value) != self.elements:
            raise ValueError(f"Value length {len(value)} does not match expected length {self.elements}")

        await self.interface.set_request({self.id: b"".join([self.type_class.encode_to_bytes(v) for v in value])})  # type: ignore


class ODINStringEntry:
    interface: TemplateInterface

    def __init__(self, id: int, max_length: int, interface: TemplateInterface):
        self.id = id
        self.interface = interface
        self.max_length = max_length

    async def read(self) -> str:
        data = await self.interface.get_single(self.id)
        if len(data) > self.max_length:
            raise ValueError(f"String length {len(data)} exceeds maximum length {self.max_length}")
        return data.decode("utf-8", errors="ignore")  # type: ignore

    async def write(self, value: str):
        if len(value) > self.max_length:
            raise ValueError(f"String length {len(value)} exceeds maximum length {self.max_length}")
        await self.interface.set_request({self.id: value.encode("utf-8")})


class ODINBytesEntry:
    interface: TemplateInterface

    def __init__(self, id: int, max_length: int, interface: TemplateInterface, fixed_length: bool):
        self.id = id
        self.interface = interface
        self.max_length = max_length
        self.fixed_length = fixed_length

    async def read(self) -> bytes:
        data = await self.interface.get_single(self.id)

        if self.fixed_length and len(data) != self.max_length:
            raise ValueError(f"Bytes length {len(data)} does not match expected length {self.max_length}")

        if len(data) > self.max_length:
            raise ValueError(f"Bytes length {len(data)} exceeds maximum length {self.max_length}")
        return data  # type: ignore

    async def write(self, value: bytes):
        if self.fixed_length and len(value) != self.max_length:
            raise ValueError(f"Bytes length {len(value)} does not match expected length {self.max_length}")

        if len(value) > self.max_length:
            raise ValueError(f"Bytes length {len(value)} exceeds maximum length {self.max_length}")
        await self.interface.set_request({self.id: value})


class ODINVectorEntry(Generic[T]):
    interface: TemplateInterface

    def __init__(self, id: int, cls: Type[GenericModel], element_size: int, max_elements: int, interface: TemplateInterface):
        self.id = id
        self.type_class = cls
        self.interface = interface
        self.element_size = element_size
        self.max_elements = max_elements

    def decode_from_bytes(self, data: bytes) -> list[T]:
        array = [self.type_class.decode_from_bytes(data[i : i + self.element_size]) for i in range(0, len(data), self.element_size)]  # type: ignore

        if len(array) > self.max_elements:
            raise ValueError(f"Value length {len(array)} exceeds maximum length {self.max_elements}")

        return array  # type: ignore

    async def read(self) -> list[T]:
        return self.decode_from_bytes(await self.interface.get_single(self.id))  # type: ignore

    async def write(self, value: list[T]):
        if len(value) > self.max_elements:
            raise ValueError(f"Value length {len(value)} exceeds maximum length {self.max_elements}")

        await self.interface.set_request({self.id: b"".join([self.type_class.encode_to_bytes(v) for v in value])})  # type: ignore


class ODINEntry(Generic[T]):
    interface: TemplateInterface

    def __init__(self, id: int, cls: Type[GenericModel], interface: TemplateInterface):
        self.id = id
        self.type_class = cls
        self.interface = interface

    async def read(self) -> T:
        return self.type_class.decode_from_bytes(await self.interface.get_single(self.id))  # type: ignore

    async def write(self, value: T):
        await self.interface.set_request({self.id: self.type_class.encode(value)})  # type: ignore


class BaseRootModel:
    _children: dict[str, "ODINEntry|ODINArrayEntry|ODINVectorEntry|BaseRootModel|ODINStringEntry|ODINBytesEntry"]

    def __init__(self, interface: TemplateInterface):
        self.interface = interface

    async def read_all(self) -> dict[str, Any]:
        # Prepare tasks for reading all children
        tasks = {}
        for name, odin_var in self._children.items():
            if isinstance(odin_var, BaseRootModel):
                tasks[name] = odin_var.read_all()
            else:
                tasks[name] = odin_var.read()

        # Execute all tasks concurrently
        results = await asyncio.gather(*tasks.values(), return_exceptions=True)

        # Combine task results into a dictionary
        data = {name: result for name, result in zip(tasks.keys(), results)}

        return data
