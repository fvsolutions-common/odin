#include "odin_utils.h"

#include <stdio.h>

#include "ansi_colors.h"
#include "odin.h"
#include "odin_codec.h"

const char *ODIN_error_to_string(int error) {
    switch (error) {
        case ODIN_SUCCESS:
            return "Success";
        case ODIN_ERROR_NO_PARAMETER:
            return "No parameter";
        case ODIN_ERROR_PARAMETER_NOT_FOUND:
            return "Parameter not found";
        case ODIN_ERROR_SIZE_MISMATCH:
            return "Size mismatch";
        case ODIN_ERROR_PERMISSION_DENIED:
            return "Permission denied";
        case ODIN_ERROR_UNSUPPORTED_FORMAT:
            return "Unsupported format";
        case ODIN_ERROR_NOT_SUPPORTED:
            return "Not supported";
        case ODIN_ERROR_BUFFER_TOO_SMALL:
            return "Buffer too small";
        case ODIN_ERROR_INVALID_PARAMETER_ACTION:
            return "Invalid parameter action";
        case ODIN_ERROR:
            return "General error";
        default:
            return "Unknown error";
    }
}

void ODIN_dump_parameter(const ODIN_parameter_t *parameter, int depth, odin_access_group_t access_group,
                         printf_like_t printf_like, const void *context) {
    char str_buffer[512];
    int ret = ODIN_encode_parameter_to_string(parameter, str_buffer, sizeof(str_buffer), access_group);

    if (ret < 0) {
        ret = sprintf(str_buffer, "Error: %s", ODIN_error_to_string(ret));
    }

    printf_like(context, ANSI_CYN "%*s%s" ANSI_RESET ": " ANSI_BLK "%s" ANSI_RESET "\n", depth, "",
                parameter->name_and_description, str_buffer);
}

void ODIN_dump_recursive_tree(const ODIN_parameter_group_t *group, int depth, odin_access_group_t access_group,
                              printf_like_t printf_like, const void *context) {
    for (int i = 0; i < group->count; i++) {
        const ODIN_parameter_generic_t *parameter = (ODIN_parameter_generic_t *)group->parameters[i];

        switch (parameter->odin_type) {
            case ODIN_TYPE_PARAMETER:
            case ODIN_TYPE_ARRAY:
            case ODIN_TYPE_VECTOR: {
                const ODIN_parameter_t *subparameter = (ODIN_parameter_t *)group->parameters[i];
                ODIN_dump_parameter(subparameter, depth, access_group, printf_like, context);
            } break;

            case ODIN_TYPE_GROUP: {
                const ODIN_parameter_group_t *sub_group = (ODIN_parameter_group_t *)group->parameters[i];
                printf_like(context,"%*s" ANSI_GRN "[%s]" ANSI_RESET "\n", depth, "", sub_group->name_and_description);
                ODIN_dump_recursive_tree(sub_group, depth + 2, access_group, printf_like, context);

            } break;
        }
    }
}