#include "codec/number_codec.h"
#include "extensions.h"
#include "odin.h"
#include "odin_security.h"

int range_validator_float(const ODIN_parameter_t *parameter, const void *data, size_t size, odin_access_group_t access_group) {
    if (parameter == NULL || data == NULL) {
        return ODIN_ERROR_INVALID_ARGUMENT;
    }

    range_parameter_t *range = (range_parameter_t *)parameter->extension->data;

    if (range == NULL) {
        return ODIN_ERROR;
    }

    float target_value;
    RETURN_ON_FAIL(ODIN_encode_data_to_float(parameter->element_type, data, size, &target_value));

    if (target_value < range->min || target_value > range->max) {
        return ODIN_ERROR_VALIDATION;
    }

    return ODIN_SUCCESS;
}

ODIN_validate_extension_ops_t ODIN_validate_extension_ops = {
    .validate = range_validator_float,
};
