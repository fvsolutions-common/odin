#include "odin_security.h"

#include "odin.h"

/**
 * @brief Method to check if a parameter is accessible
 *
 * @param parameter Pointer to the parameter
 * @param access_group The access group, which is requesting the access
 * @param operation The operation, which is requested
 *
 * @return true if the access is allowed, false otherwise
 */
bool ODIN_check_access(const ODIN_parameter_t *parameter, odin_access_group_t access_group, odin_acces_operation_t operation) {
    // Check if the parameter is valid
    if (parameter == NULL) {
        return false;
    }

    // Internal access is always allowed
    if (access_group == ODIN_ACCESS_GROUP_INTERNAL) {
        return true;
    }

    // Check if the operation is allowed
    if ((parameter->flags & operation & access_group) == 0) {
        return false;
    }

    return true;
}

/**
 * @brief Method to validate the access to a parameter
 * It will log an error if the access is denied
 *
 * @param parameter Pointer to the parameter
 * @param access_group The access group, which is requesting the access
 * @param operation The operation, which is requested
 *
 * @return if negative an error code is returned, otherwise ODIN_SUCCESS
 */
odin_error_t ODIN_validate_access(const ODIN_parameter_t *parameter, odin_access_group_t access_group,
                                  odin_acces_operation_t operation) {
    // Checks access to the parameter, logs an error if access is denied
    if (!ODIN_check_access(parameter, access_group, operation)) {
        printf( "Permission denied when reading %s", parameter->name_and_description);
        return ODIN_ERROR_PERMISSION_DENIED;
    }

    return ODIN_SUCCESS;
}