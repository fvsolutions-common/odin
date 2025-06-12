from typing import Any
import csnake as cc

from ....data_types.type_registry import BuiltinDataType, CharCDataType, CustomDataType, DataType, BooleanCDataType
from ....parameter import (
    ArrayParameterModel,
    BaseBaseParameterModel,
    BaseParameterGroupModel,
    BaseParameterModel,
    ParameterGroupModel,
    ParameterModel,
    RootParameterModel,
    VectorParameterModel,
)
from ....utils.csnake_custom import StructVariable


def type_to_c_initialiser(type: DataType, name: str, default: str | None) -> cc.Variable:
    if isinstance(type, BooleanCDataType):
        assert default is not None, "Default value must be provided"
        if default:
            default = "true"
        else:
            default = "false"

    if isinstance(type, CharCDataType):
        assert default is not None, "Default value must be provided"

        return cc.Variable(
            name=name,
            primitive=type.c_typename,
            value=cc.TextModifier(f"'{default}'"),  # type: ignore
        )

    elif isinstance(type, BuiltinDataType):
        assert default is not None, f"Default value must be provided, because {type.typename} does not provide a default value"
        return cc.Variable(
            name=name,
            primitive=type.c_typename,
            value=cc.TextModifier(str(default)),  # type: ignore
        )

    elif isinstance(type, CustomDataType):
        if default is None:
            default = type.default

        assert default is not None, "Default value must be provided, either in the parameter or in the type definition"

        return cc.Variable(
            name=name,
            primitive=type.c_typename,
            value=default,  # type: ignore
        )

    else:
        # def to_c_initialiser(self, name: str, default: Any) -> cc.Variable:
        raise NotImplementedError("This function is not implemented yet")


def type_to_c_array_initialiser(
    type: DataType,
    name: str,
    defaults: list[Any] | str | None,
    dimensions: list[int],
) -> cc.Variable:
    if isinstance(type, BooleanCDataType) and defaults:
        defaults = ["true" if x else "false" for x in defaults]

    if isinstance(type, CharCDataType):
        if isinstance(defaults, str):
            value = cc.TextModifier(f'"{defaults}"')  # type: ignore
        elif defaults is None:
            value = [cc.TextModifier("0")]  # type: ignore
        else:
            value = [cc.TextModifier(f"'{x}'") for x in defaults]  # type: ignore
        return cc.Variable(
            name=name,
            primitive=type.c_typename,
            array=dimensions,
            value=value,  # type: ignore
        )

    elif isinstance(type, BuiltinDataType):
        return cc.Variable(
            name=name,
            primitive=type.c_typename,
            array=dimensions,
            value=[cc.TextModifier(str(x)) for x in defaults],  # type: ignore
        )

    elif isinstance(type, CustomDataType):
        return cc.Variable(
            name=name,
            primitive=type.c_typename,
            array=dimensions,
            value=defaults,  # type: ignore
        )

    else:
        raise NotImplementedError("This function is not implemented yet")


def to_variable_initialiser(parameter: BaseParameterModel) -> cc.Variable | None:
    if isinstance(parameter, BaseBaseParameterModel):
        # If the reference is set, then we don't need to initialise the variable
        if parameter.reference:
            return None

        if isinstance(parameter, ParameterModel):
            # If the type is void, then we also don't need to initialise the variable
            if parameter.type == "void":
                return None

            return type_to_c_initialiser(
                type=parameter._resolved_type,
                name=parameter._name,
                default=parameter.default,
            )

        elif isinstance(parameter, ArrayParameterModel):
            assert isinstance(parameter._resolved_type, BuiltinDataType)
            return type_to_c_array_initialiser(
                type=parameter._resolved_type,
                name=parameter._name,
                defaults=parameter.default if parameter.default else [],
                dimensions=[parameter.elements],
            )

        elif isinstance(parameter, VectorParameterModel):
            data = type_to_c_array_initialiser(
                type=parameter._resolved_type,
                name=parameter._name,
                defaults=parameter.default,
                dimensions=[parameter.max_elements],
            )

            length = len(parameter.default) if parameter.default else 0

            return cc.Variable(
                name=parameter._name,
                primitive="UNKNOWN PRIMITIVE",
                value={"data": data.value, "num_elements": length},  # type: ignore
            )

    elif isinstance(parameter, BaseParameterGroupModel):
        if isinstance(parameter, RootParameterModel):
            assert parameter._c_types is not None, "C types are not set"
            name = parameter._c_types.variables_name
            type = parameter._c_types.variables_type
        else:
            name = parameter._name
            type = "UNKNOWN PRIMITIVE"

        values = {}
        for child in parameter.children.values():
            initialiser = to_variable_initialiser(child)

            if initialiser:
                values[initialiser.name] = initialiser.value

        if len(values) == 0:
            return None

        return cc.Variable(name=name, primitive=type, value=values)  # type: ignore


def to_variable_type(parameter: BaseParameterModel, type_name: str | None = None) -> cc.Struct | None:
    if isinstance(parameter, VectorParameterModel):
        if type_name:
            struct = StructVariable(name=type_name, typedef=True)
        else:
            struct = StructVariable(var_name=parameter._name, typedef=False)

        struct.add_variable(
            cc.Variable(
                name="num_elements",
                primitive="size_t",
                value=len(parameter.default) if parameter.default else 0,  # type: ignore
            ),
        )

        struct.add_variable(
            type_to_c_array_initialiser(
                type=parameter._resolved_type,
                name="data",
                defaults=parameter.default,
                dimensions=[parameter.max_elements],
            )
        )

        return struct

    elif isinstance(parameter, BaseParameterGroupModel):
        if isinstance(parameter, RootParameterModel):
            assert parameter._c_types is not None, "C types are not set"
            struct = StructVariable(name=parameter._c_types.variables_type, typedef=True)
        else:
            struct = StructVariable(var_name=parameter._name, typedef=False)

        items = 0

        for child in parameter.children.values():
            if isinstance(child, ParameterGroupModel):
                variable_type = to_variable_type(child)
                if variable_type:
                    struct.add_struct(variable_type)
                    items += 1

            elif isinstance(child, VectorParameterModel):
                to_variable_initialiser(child)
                variable_type = to_variable_type(child)
                if variable_type:
                    struct.add_struct(variable_type)
                    items += 1

            elif isinstance(child, ParameterModel) or isinstance(child, ArrayParameterModel):
                variable = to_variable_initialiser(child)
                if variable:
                    struct.add_variable(variable)
                    items += 1

            else:
                raise ValueError("Unknown type")

        if items == 0:
            return None

        return struct

    else:
        raise ValueError("Unknown type")
