import csnake as cc

from ....generators.c.model import ODIN_ArrayModel, ODIN_ParameterModel
from ....parameter import (
    ArrayParameterModel,
    BaseBaseParameterModel,
    BaseParameterGroupModel,
    ParameterGroupModel,
    ParameterModel,
    RootParameterModel,
    VectorParameterModel,
)
from ....utils.csnake_custom import StructVariable


def to_object_initialiser(parameter) -> cc.Variable:
    if isinstance(parameter, BaseBaseParameterModel):
        # If the reference is set, then we don't need to initialise the variable
        if parameter.reference:
            parameter._absolute_variable_reference = parameter.reference

        data_address = parameter._absolute_variable_reference

        if isinstance(parameter, ParameterModel):
            """Returns the odin C initialiser for the parameter"""

            formatted_extension = None
            if len(parameter.extensions) > 0:
                next = None
                for extension in parameter.extensions:
                    formatted_extension = extension.as_literal(parameter, next)
                    next = formatted_extension

            if parameter.type == "void":
                data_address = None

            return ODIN_ParameterModel(
                type=parameter._resolved_type,
                access_group=parameter.access_control.collapse().to_c_definition(),
                local_index=parameter.local_id,
                global_index=parameter.global_id,
                data=data_address,
                name=parameter._name,
                description=parameter.resolved_description,
                extensions=formatted_extension,
            ).variable

        elif isinstance(parameter, ArrayParameterModel):
            return ODIN_ArrayModel(
                type=parameter._resolved_type,
                access_group=parameter.access_control.collapse().to_c_definition(),
                local_index=parameter.local_id,
                global_index=parameter.global_id,
                data=data_address,
                name=parameter._name,
                description=parameter.resolved_description,
                num_elements=parameter.elements,
            ).variable

        elif isinstance(parameter, VectorParameterModel):
            return ODIN_ArrayModel(
                type=parameter._resolved_type,
                access_group=parameter.access_control.collapse().to_c_definition(),
                local_index=parameter.local_id,
                global_index=parameter.global_id,
                data=data_address,
                name=parameter._name,
                description=parameter.resolved_description,
                num_elements=parameter.max_elements,
                fixed_size=False,
            ).variable
        else:
            raise ValueError("Unknown type")

    elif isinstance(parameter, BaseParameterGroupModel):
        if isinstance(parameter, RootParameterModel):
            assert parameter._c_types is not None, "C types are not set"
            name = parameter._c_types.objects_name
            type = parameter._c_types.objects_type
        else:
            name = parameter._name
            type = "UNKNOWN PRIMITIVE"

        return cc.Variable(
            name=name,
            primitive=type,
            value={
                parameter_name: to_object_initialiser(parameter).value
                for parameter_name, parameter in parameter.children.items()  # type: ignore
            },
        )

    else:
        raise ValueError("Unknown type")


def to_object_type(
    group: BaseParameterGroupModel,
) -> cc.Struct:
    if isinstance(group, RootParameterModel):
        assert group._c_types is not None, "C types are not set"
        struct = StructVariable(name=group._c_types.objects_type, typedef=True)

    else:
        struct = StructVariable(var_name=group._name, typedef=False)

    # Create the data structures for groups
    for child in group.children.values():
        if isinstance(child, ParameterGroupModel):
            struct.add_struct(to_object_type(child))

        elif isinstance(child, ParameterModel) or isinstance(child, VectorParameterModel) or isinstance(child, ArrayParameterModel):
            struct.add_variable(to_object_initialiser(child))

        else:
            raise ValueError("Unknown type")

    return struct
