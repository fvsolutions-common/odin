#include "odin.h"
#include "stdint.h"

#ifndef ODIN_SECURITY_H
#define ODIN_SECURITY_H

#define ODIN_ACCES_SIZE_BITS 3
#define ODIN_ACCESS_MAX ((1 << (ODIN_ACCES_SIZE_BITS)) - 1)
#define GROUP_MASK(group) (ODIN_ACCESS_MAX << (ODIN_ACCES_SIZE_BITS * group))

#define DERIVE_ACCESS(bit) (1 << (bit))

// We support 6 access groups
typedef enum odin_acces_group{
    ODIN_ACCESS_GROUP_INTERNAL = 0,  // Internal access group
    ODIN_ACCESS_GROUP_0 = GROUP_MASK(0),
    ODIN_ACCESS_GROUP_1 = GROUP_MASK(1),
    ODIN_ACCESS_GROUP_2 = GROUP_MASK(2),
    ODIN_ACCESS_GROUP_3 = GROUP_MASK(3),
    ODIN_ACCESS_GROUP_4 = GROUP_MASK(4),
    ODIN_ACCESS_GROUP_5 = GROUP_MASK(5),
} odin_access_group_t;

// static int access = DERIVE_ACCESS(ODIN_ACCES_SIZE_BITS);

#define DERIVE_ACCESS_T(base, mul) (base << (ODIN_ACCES_SIZE_BITS * mul))

#define ADD_GROUPS(access)                                                                            \
    ((access | DERIVE_ACCESS_T(access, 0) | DERIVE_ACCESS_T(access, 1) | DERIVE_ACCESS_T(access, 2) | \
      DERIVE_ACCESS_T(access, 3) | DERIVE_ACCESS_T(access, 4) | DERIVE_ACCESS_T(access, 5)))

typedef enum {
    ODIN_ACCESS_READ = ADD_GROUPS(0x1),   // Ability to read the parameter
    ODIN_ACCESS_WRITE = ADD_GROUPS(0x2),  // Ability to write the parameter
    // ODIN_ACCESS_RESET = 0x10, //Ability to reset the parameter to its default value
    // ODIN_ACCESS_LOG_READ = 0x4,   // If set read access will be logged if the group is also set to log the access
    ODIN_ACCESS_LOG_WRITE = ADD_GROUPS(0x4),  // If set write access will be logged if the group is also set to log the access
} odin_acces_operation_t;

bool ODIN_check_access(const ODIN_parameter_t *parameter, odin_access_group_t access_group, odin_acces_operation_t operation);
odin_error_t ODIN_validate_access(const ODIN_parameter_t *parameter, odin_access_group_t access_group,
                                  odin_acces_operation_t operation);

#endif  // ODIN_SECURITY_H
