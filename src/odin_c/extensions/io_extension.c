

#include "codec/number_codec.h"
#include "extensions.h"
#include "odin.h"
#include "odin_security.h"

static int read_operation(const ODIN_parameter_t *parameter, void *data, size_t size, odin_access_group_t access_group) {
    if (parameter == NULL || data == NULL) {
        return ODIN_ERROR_INVALID_ARGUMENT;
    }

    mapped_number_parameters_t *config = (mapped_number_parameters_t *)parameter->extension->data;

    // Get number from refernce parameter
    float value;
    RETURN_ON_FAIL(ODIN_encode_data_to_float(config->reference->element_type, config->reference->data,
                                             config->reference->element_size, &value));

    // Scale and offset
    value = value * config->scale + config->offset;

    // Write to the current parameter
    RETURN_ON_FAIL(ODIN_decode_float_to_data(parameter->element_type, data, size, value));

    return size;
}

static int write_operation(const ODIN_parameter_t *parameter, const void *data, size_t size, odin_access_group_t access_group) {
    if (parameter == NULL || data == NULL) {
        return ODIN_ERROR_INVALID_ARGUMENT;
    }

    mapped_number_parameters_t *config = (mapped_number_parameters_t *)parameter->extension->data;

    // Get number from refernce parameter
    float value;
    RETURN_ON_FAIL(ODIN_encode_data_to_float(parameter->element_type, data, size, &value));

    // Scale and offset
    value = (value - config->offset) / config->scale;

    // Write to the current parameter
    RETURN_ON_FAIL(ODIN_decode_float_to_data(config->reference->element_type, config->reference->data, config->reference->element_size,
                                     value));
    return size;
}

ODIN_io_extension_ops_t ODIN_extension_io_mapped_number_ops = {
    .write = write_operation,
    .read = read_operation,
};
