from odin_db import (
    ODINDBModelType,
    OdinDBParameterGroupModel,
    OdinDBParameterModel,
    OdinDBTypeDefinitionModel,
    OdinDBArrayModel,
    OdinDBVectorModel,
)

from odin_python.parameter.parameter import VectorParameterModel, ArrayParameterModel, BaseBaseParameterModel

from ...data_types.type_registry import CustomDataType, DataType, DataTypeModelDefinition
from ...parameter import BaseParameterGroupModel, ParameterGroupModel, ParameterModel


def parameter_to_db(parameter: BaseBaseParameterModel) -> OdinDBParameterModel | OdinDBArrayModel | OdinDBVectorModel:
    if isinstance(parameter, ArrayParameterModel):
        if isinstance(parameter.default, str):
            default_value = list(parameter.default)
        else:
            default_value = parameter.default

        return OdinDBArrayModel(
            name=parameter._name,
            description=parameter.resolved_description,
            global_id=parameter.global_id,
            global_name=parameter.global_name,
            element_size=parameter._resolved_type.size,
            element_type=parameter._resolved_type.typename,
            default_value=default_value,
            element_count=parameter.elements,
        )
    elif isinstance(parameter, VectorParameterModel):
        if isinstance(parameter.default, str):
            default_value = list(parameter.default)
        else:
            default_value = parameter.default

        return OdinDBVectorModel(
            name=parameter._name,
            description=parameter.resolved_description,
            global_id=parameter.global_id,
            global_name=parameter.global_name,
            element_size=parameter._resolved_type.size,
            element_type=parameter._resolved_type.typename,
            default_value=default_value,
            max_element_count=parameter.max_elements,
        )

    elif isinstance(parameter, ParameterModel):
        return OdinDBParameterModel(
            name=parameter._name,
            description=parameter.resolved_description,
            global_id=parameter.global_id,
            global_name=parameter.global_name,
            element_size=parameter._resolved_type.size,
            element_type=parameter._resolved_type.typename,
            default_value=parameter.default,
        )
    else:
        raise ValueError(f"Unknown parameter type: {parameter}")


def parameter_group_to_db(group: BaseParameterGroupModel) -> OdinDBParameterGroupModel:
    parameters = []
    for name, child in group.children.items():
        if isinstance(child, BaseBaseParameterModel):
            parameters.append(parameter_to_db(child))

        elif isinstance(child, ParameterGroupModel):
            parameters.append(parameter_group_to_db(child))

    return OdinDBParameterGroupModel(
        name=group._name,
        global_id=group.global_id,
        global_name=group.global_name,
        description=group.resolved_description,
        parameters=parameters,
    )


def type_to_odin_db(type: DataTypeModelDefinition) -> OdinDBTypeDefinitionModel:
    if isinstance(type.root, DataType):
        if isinstance(type.root, CustomDataType):
            return type_to_odin_db(type.root.model)

        elif isinstance(type.root, DataType):
            return OdinDBTypeDefinitionModel(
                count=type.elements,
                size=type.size,
                structure=ODINDBModelType.from_string(type.root.typename),
            )
        else:
            raise ValueError(f"Unknown data type: {type.root}")

    elif isinstance(type.root, dict):
        structure_item: dict[str, OdinDBTypeDefinitionModel] = {}
        for name, child in type.root.items():
            if isinstance(child, DataTypeModelDefinition):
                structure_item[name] = type_to_odin_db(child)
                structure_item[name].count = child.elements

            else:
                raise ValueError(f"Unknown data type: {child}")

        return OdinDBTypeDefinitionModel(
            size=type.size,
            structure=structure_item,  # type: ignore
            count=type.elements,
        )

    else:
        raise ValueError(f"Unknown data type: {type.root}")
