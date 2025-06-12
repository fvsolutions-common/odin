from enum import Enum


class ODIN_ExtensionEnum(Enum):
    ODIN_EXTENSION_TYPE_IO = "ODIN_EXTENSION_TYPE_IO"
    ODIN_EXTENSION_TYPE_VALIDATE = "ODIN_EXTENSION_TYPE_VALIDATE"
    ODIN_EXTENSION_TYPE_STRING_CODEC = "ODIN_EXTENSION_TYPE_STRING_CODEC"


class ODIN_TypeEnum(Enum):
    ODIN_TYPE_PARAMETER = "ODIN_TYPE_PARAMETER"
    ODIN_TYPE_ARRAY = "ODIN_TYPE_ARRAY"
    ODIN_TYPE_VECTOR = "ODIN_TYPE_VECTOR"
    ODIN_TYPE_GROUP = "ODIN_TYPE_GROUP"


class ODIN_ElementTypeEnum(Enum):
    ODIN_ELEMENT_TYPE_BOOL = "ODIN_ELEMENT_TYPE_BOOL"
    ODIN_ELEMENT_TYPE_HEX = "ODIN_ELEMENT_TYPE_HEX"
    ODIN_ELEMENT_TYPE_UINT8 = "ODIN_ELEMENT_TYPE_UINT8"
    ODIN_ELEMENT_TYPE_UINT16 = "ODIN_ELEMENT_TYPE_UINT16"
    ODIN_ELEMENT_TYPE_UINT32 = "ODIN_ELEMENT_TYPE_UINT32"
    ODIN_ELEMENT_TYPE_UINT64 = "ODIN_ELEMENT_TYPE_UINT64"
    ODIN_ELEMENT_TYPE_INT8 = "ODIN_ELEMENT_TYPE_INT8"
    ODIN_ELEMENT_TYPE_INT16 = "ODIN_ELEMENT_TYPE_INT16"
    ODIN_ELEMENT_TYPE_INT32 = "ODIN_ELEMENT_TYPE_INT32"
    ODIN_ELEMENT_TYPE_INT64 = "ODIN_ELEMENT_TYPE_INT64"
    ODIN_ELEMENT_TYPE_FLOAT32 = "ODIN_ELEMENT_TYPE_FLOAT32"
    ODIN_ELEMENT_TYPE_FLOAT64 = "ODIN_ELEMENT_TYPE_FLOAT64"
    ODIN_ELEMENT_TYPE_CHAR = "ODIN_ELEMENT_TYPE_CHAR"
    ODIN_ELEMENT_TYPE_CUSTOM = "ODIN_ELEMENT_TYPE_CUSTOM"

    @classmethod
    def from_c_type(cls, c_type: str) -> "ODIN_ElementTypeEnum":
        if c_type == "bool":
            return cls.ODIN_ELEMENT_TYPE_BOOL
        if c_type == "uint8_t":
            return cls.ODIN_ELEMENT_TYPE_UINT8
        if c_type == "uint16_t":
            return cls.ODIN_ELEMENT_TYPE_UINT16
        if c_type == "uint32_t":
            return cls.ODIN_ELEMENT_TYPE_UINT32
        if c_type == "uint64_t":
            return cls.ODIN_ELEMENT_TYPE_UINT64
        if c_type == "int8_t":
            return cls.ODIN_ELEMENT_TYPE_INT8
        if c_type == "int16_t":
            return cls.ODIN_ELEMENT_TYPE_INT16
        if c_type == "int32_t":
            return cls.ODIN_ELEMENT_TYPE_INT32
        if c_type == "int64_t":
            return cls.ODIN_ELEMENT_TYPE_INT64
        if c_type == "float":
            return cls.ODIN_ELEMENT_TYPE_FLOAT32
        if c_type == "double":
            return cls.ODIN_ELEMENT_TYPE_FLOAT64
        if c_type == "char":
            return cls.ODIN_ELEMENT_TYPE_CHAR

        return cls.ODIN_ELEMENT_TYPE_CUSTOM
