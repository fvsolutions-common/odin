#include "odin.h"
#include "odin_security.h"
#include "cJSON.h"

int ODIN_encode_parameter_group_to_JSON(const ODIN_parameter_group_t *group,
                                        cJSON *object,
                                        odin_access_group_t access_group);