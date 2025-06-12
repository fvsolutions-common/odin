#include "odin.h"

const ODIN_parameter_generic_t *ODIN_get_generic_parameter_by_id(const ODIN_parameter_group_t *group, uint32_t index,
                                                                 uint32_t parent_shift);
const ODIN_parameter_t *ODIN_get_parameter_by_id(const ODIN_parameter_group_t *group, uint32_t index, uint32_t parent_shift);
const ODIN_parameter_group_t *ODIN_get_parameter_group_by_id(const ODIN_parameter_group_t *group, uint32_t index,
                                                             uint32_t parent_shift);
const ODIN_parameter_t *ODIN_get_parameter_by_name(const ODIN_parameter_group_t *group, const char *name, char separator);
const ODIN_parameter_group_t *ODIN_get_group_by_name(const ODIN_parameter_group_t *group, const char *name, char separator);
const ODIN_parameter_generic_t *ODIN_get_generic_parameter_by_name(const ODIN_parameter_group_t *group, const char *name,
                                                                   char separator);