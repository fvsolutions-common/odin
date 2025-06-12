
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "inttypes.h"
#include "odin.h"
#include "odin_codec.h"
#include "odin_core.h"
#include "odin_lookup.h"

// static const char *TAG = "ODIN_STR";

#define ODIN_EXPORT_BUFFER_SIZE 128

/**
 * @brief Encoding structure
 * 32 bit index
 * 16 bit length
 * data (variable length)
 */
#pragma pack(push, 1)
typedef struct {
    uint32_t index;
    uint16_t length;
    uint8_t data[];
} byte_package_format_t;
#pragma pack(pop)

/**
 * @brief Encode a parameter to a byte buffer, see @ref byte_package_format_t for the format
 *
 * @param parameter The parameter to encode
 * @param output_buffer The buffer to write the encoded parameter to
 * @param output_buffer_size The size of the output buffer
 * @param access_group The access group to use
 *
 * @return int The size of the encoded parameter or an error code
 */
int ODIN_encode_parameter_to_bytes(const ODIN_parameter_t *parameter, uint8_t *output_buffer, size_t output_buffer_size,
                                   odin_access_group_t access_group) {
    // Check if the group and buffer are valid
    if (parameter == NULL || output_buffer == NULL) {
        return ODIN_ERROR_INVALID_ARGUMENT;
    }

    // Check if the buffer is large enough
    if (output_buffer_size < sizeof(byte_package_format_t)) {
        return ODIN_ERROR_SIZE_MISMATCH;
    }

    byte_package_format_t *package = (byte_package_format_t *)output_buffer;
    size_t data_buffer_size = output_buffer_size - sizeof(byte_package_format_t);

    int size = ODIN_parameter_read_into_buffer(parameter, package->data, data_buffer_size, access_group);

    if (size < ODIN_SUCCESS) {
        return size;
    }

    package->index = parameter->global_index;
    package->length = size;

    return sizeof(byte_package_format_t) + size;
}

/**
 * @brief Decode a byte buffer to a parameter, see @ref byte_package_format_t for the format
 *
 * @param group The parameter group to use
 * @param input_buffer The buffer to read the encoded parameter from
 * @param input_buffer_size The size of the input buffer
 * @param access_group The access group to use
 *
 * @return int An error code
 */
int ODIN_decode_bytes_to_parameter(const ODIN_parameter_group_t *group, const uint8_t *input_buffer,
                                   size_t input_buffer_data_length, odin_access_group_t access_group) {
    // Check if the group and buffer are valid
    if (group == NULL || input_buffer == NULL) {
        return ODIN_ERROR_INVALID_ARGUMENT;
    }

    // Check we have at least the header
    if (input_buffer_data_length < sizeof(byte_package_format_t)) {
        return ODIN_ERROR_SIZE_MISMATCH;
    }

    byte_package_format_t *package = (byte_package_format_t *)input_buffer;

    // Check if the lenth is correct
    if (input_buffer_data_length < package->length + sizeof(byte_package_format_t)) {
        return ODIN_ERROR_SIZE_MISMATCH;
    }

    const ODIN_parameter_t *parameter = ODIN_get_parameter_by_id(group, package->index, 0);
    if (parameter == NULL) {
        return ODIN_ERROR_PARAMETER_NOT_FOUND;
    }

    return ODIN_parameter_write(parameter, package->data, package->length, access_group);
}

/**
 * @brief Recursively encode a parameter group to a byte buffer
 *
 * @param group The group to encode
 * @param output_buffer The buffer to write the encoded group to
 * @param output_buffer_size The size of the output buffer
 * @param access_group The access group to use
 *
 * @return int The size of the encoded group or an error code
 */
int ODIN_encode_parameter_group_to_bytes(const ODIN_parameter_group_t *group, uint8_t *output_buffer, size_t output_buffer_size,
                                         odin_access_group_t access_group) {
    // Check if the group and buffer are valid
    if (group == NULL || output_buffer == NULL) {
        return ODIN_ERROR_INVALID_ARGUMENT;
    }

    uint8_t *output_buffer_start = output_buffer;
    for (int i = 0; i < group->count; i++) {
        // Check type of parameter, if group, call recursively
        const ODIN_parameter_generic_t *parameter = (const ODIN_parameter_generic_t *)group->parameters[i];

        if (parameter->odin_type == ODIN_TYPE_GROUP) {
            // Encode the group
            int len = ODIN_encode_parameter_group_to_bytes((const ODIN_parameter_group_t *)parameter, output_buffer,
                                                           output_buffer_size, access_group);
            if (len < ODIN_SUCCESS) {
                return len;
            }

            output_buffer += len;
            output_buffer_size -= len;
        } else {
            // Encode the parameter
            int len = ODIN_encode_parameter_to_bytes((const ODIN_parameter_t *)parameter, output_buffer, output_buffer_size,
                                                     access_group);
            if (len < ODIN_SUCCESS) {
                return len;
            }

            output_buffer += len;
            output_buffer_size -= len;
        }
    }

    return output_buffer - output_buffer_start;
}

/**
 * @brief Recursively decode a byte buffer to a parameter group
 *
 * @param group The group to decode into
 * @param input_buffer The buffer to read the encoded group from
 * @param input_buffer_size The size of the input buffer
 * @param access_group The access group to use
 *
 * @return int The size of the decoded group or an error code
 */
int ODIN_decode_bytes_to_parameter_group(const ODIN_parameter_group_t *group, const uint8_t *input_buffer,
                                         size_t input_buffer_data_length, odin_access_group_t access_group) {
    // Check if the group and buffer are valid
    if (group == NULL || input_buffer == NULL) {
        return ODIN_ERROR_INVALID_ARGUMENT;
    }

    const uint8_t *input_buffer_start = input_buffer;
    while (input_buffer - input_buffer_start < input_buffer_data_length) {
        // Check if we have enough data for the header
        if (input_buffer - input_buffer_start + sizeof(byte_package_format_t) > input_buffer_data_length) {
            return ODIN_ERROR_SIZE_MISMATCH;
        }

        byte_package_format_t *package = (byte_package_format_t *)input_buffer;

        // Check if the lenth is correct, to fit in the remaining buffer
        if (input_buffer - input_buffer_start + package->length + sizeof(byte_package_format_t) > input_buffer_data_length) {
            return ODIN_ERROR_SIZE_MISMATCH;
        }

        // Find the parameter
        const ODIN_parameter_t *parameter = ODIN_get_parameter_by_id(group, package->index, 0);
        if (parameter == NULL) {
            return ODIN_ERROR_PARAMETER_NOT_FOUND;
        }

        // Write the parameter
        int ret = ODIN_parameter_write(parameter, package->data, package->length, access_group);
        if (ret < ODIN_SUCCESS) {
            return ret;
        }

        input_buffer += sizeof(byte_package_format_t) + package->length;
    }

    return input_buffer - input_buffer_start;
}