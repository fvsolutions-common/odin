#include <stddef.h>

#include "odin.h"
#include "odin_security.h"

// Type printf function like
typedef void (*printf_like_t)(const void *context, const char *fmt, ...);

const char *ODIN_error_to_string(int error);
void ODIN_dump_parameter(const ODIN_parameter_t *parameter, int depth, odin_access_group_t access_group,
                           printf_like_t printf_like, const void *context);
void ODIN_dump_recursive_tree(const ODIN_parameter_group_t *group, int depth, odin_access_group_t access_group,
                               printf_like_t printf_like, const void *context);
