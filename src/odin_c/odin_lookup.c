#include "odin_lookup.h"

#include <string.h>

#include "odin.h"

const ODIN_parameter_generic_t *ODIN_get_generic_parameter_by_id(const ODIN_parameter_group_t *group, uint32_t index,
                                                                 uint32_t parent_shift) {
    if (group == NULL) {
        return NULL;
    }

    int shift = group->shift + parent_shift;

    uint32_t mask = ((1 << shift) - 1) << (32 - shift);
    for (int parameter_idx = 0; parameter_idx < group->count; parameter_idx++) {
        ODIN_parameter_generic_t *parameter = (ODIN_parameter_generic_t *)group->parameters[parameter_idx];

        // Check if the mask matches
        if ((parameter->global_index & mask) != (index & mask)) {
            continue;
        }

        // If we have an exact match, return the parameter
        if (parameter->global_index == index) {
            return parameter;
        }

        // If the parameter is a group, we need to check the children of the group
        if (parameter->odin_type == ODIN_TYPE_GROUP) {
            const ODIN_parameter_generic_t *param =
                ODIN_get_generic_parameter_by_id((ODIN_parameter_group_t *)parameter, index, shift);
            if (param != NULL) {
                return param;
            }
        }
    }
    return NULL;
}

const ODIN_parameter_t *ODIN_get_parameter_by_id(const ODIN_parameter_group_t *group, uint32_t index, uint32_t parent_shift) {
    const ODIN_parameter_generic_t *parameter = ODIN_get_generic_parameter_by_id(group, index, parent_shift);
    if (parameter == NULL) {
        return NULL;
    }
    if (parameter->odin_type == ODIN_TYPE_GROUP) {
        return NULL;
    }

    return (ODIN_parameter_t *)parameter;
}

const ODIN_parameter_group_t *ODIN_get_parameter_group_by_id(const ODIN_parameter_group_t *group, uint32_t index,
                                                             uint32_t parent_shift) {
    const ODIN_parameter_generic_t *parameter = ODIN_get_generic_parameter_by_id(group, index, parent_shift);
    if (parameter == NULL) {
        return NULL;
    }
    if (parameter->odin_type != ODIN_TYPE_GROUP) {
        return NULL;
    }

    return (ODIN_parameter_group_t *)parameter;
}

const ODIN_parameter_generic_t *ODIN_get_generic_parameter_by_name(const ODIN_parameter_group_t *group, const char *name,
                                                                   char separator) {
    if (group == NULL) {
        return NULL;
    }

    // Find the first separator
    const char *sub_string = strchr(name, separator);
    if (sub_string == NULL) {
        // No separator, this is the last element
        for (int parameter_idx = 0; parameter_idx < group->count; parameter_idx++) {
            ODIN_parameter_generic_t *generic_parameter = (ODIN_parameter_generic_t *)group->parameters[parameter_idx];
            // if group
            if (generic_parameter->odin_type == ODIN_TYPE_GROUP) {
                ODIN_parameter_group_t *subgroup = (ODIN_parameter_group_t *)generic_parameter;
                if (strcmp(subgroup->name_and_description, name) == 0) {
                    return generic_parameter;
                }
            } else {
                ODIN_parameter_t *parameter = (ODIN_parameter_t *)generic_parameter;
                if (strcmp(parameter->name_and_description, name) == 0) {
                    return generic_parameter;
                }
            }
        }
        return NULL;
    }

    // Find the group
    for (int parameter_idx = 0; parameter_idx < group->count; parameter_idx++) {
        ODIN_parameter_generic_t *parameter = (ODIN_parameter_generic_t *)group->parameters[parameter_idx];
        if (parameter->odin_type != ODIN_TYPE_GROUP) {
            continue;
        }

        const ODIN_parameter_group_t *sub_group = (ODIN_parameter_group_t *)parameter;
        if (strncmp(sub_group->name_and_description, name, sub_string - name) == 0) {
            return ODIN_get_generic_parameter_by_name(sub_group, sub_string + 1, separator);
        }
    }

    return NULL;
}

const ODIN_parameter_t *ODIN_get_parameter_by_name(const ODIN_parameter_group_t *group, const char *name, char separator) {
    const ODIN_parameter_generic_t *parameter = ODIN_get_generic_parameter_by_name(group, name, separator);
    if (parameter == NULL || parameter->odin_type == ODIN_TYPE_GROUP) {
        return NULL;
    }
    return (ODIN_parameter_t *)parameter;
}

const ODIN_parameter_group_t *ODIN_get_group_by_name(const ODIN_parameter_group_t *group, const char *name, char separator) {
    const ODIN_parameter_generic_t *parameter = ODIN_get_generic_parameter_by_name(group, name, separator);
    if (parameter == NULL || parameter->odin_type != ODIN_TYPE_GROUP) {
        return NULL;
    }
    return (ODIN_parameter_group_t *)parameter;
}
