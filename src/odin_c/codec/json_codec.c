#include "json_codec.h"

int ODIN_encode_parameter_group_to_JSON(const ODIN_parameter_group_t *group,
                                        cJSON *object,
                                        odin_access_group_t access_group) {
  // Check if the group and buffer are valid
  if (group == NULL || object == NULL) {
    return ODIN_ERROR_INVALID_ARGUMENT;
  }
  for (int i = 0; i < group->count; i++) {
    // Check type of parameter, if group, call recursively
    const ODIN_parameter_generic_t *generic_parameter =
        (const ODIN_parameter_generic_t *)group->parameters[i];

    if (generic_parameter->odin_type == ODIN_TYPE_GROUP) {
      // Create a new object
      cJSON *sub_object = cJSON_CreateObject();
      const ODIN_parameter_group_t *group =
          (const ODIN_parameter_group_t *)generic_parameter;
      // Encode the group
      int len = ODIN_encode_parameter_group_to_JSON(
          (const ODIN_parameter_group_t *)group, sub_object, access_group);

      if (len < ODIN_SUCCESS) {
        // destroy the object
        cJSON_Delete(sub_object);

        return len;
      }

      // add the object to the parent object
      cJSON_AddItemToObject(object, group->name_and_description, sub_object);

    } else {
      const ODIN_parameter_t *parameter =
          (const ODIN_parameter_t *)generic_parameter;

      // Just add the name and index to the object for now
      cJSON_AddNumberToObject(object, parameter->name_and_description,
                              parameter->global_index);
    }
  }
  return ODIN_SUCCESS;
}