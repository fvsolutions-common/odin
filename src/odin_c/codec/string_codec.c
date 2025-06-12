
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "extensions/extensions.h"
#include "inttypes.h"
#include "odin_codec.h"
#include "odin_core.h"

// static const char *TAG = "ODIN_STR";

#define ODIN_PRINT_BUFFER_SIZE 128

/**
 * @brief Serialise an element to a human readable string
 * Note does not handle the custom string encoding
 *
 * @param element_type Type of the element
 * @param data Pointer to the data
 * @param output_buffer Pointer to the buffer to store the string
 * @param output_buffer_size Size of the buffer
 *
 * @return int Length of the string
 */
int ODIN_element_to_string(const ODIN_parameter_t *parameter, void *data, char *output_buffer, size_t output_buffer_size) {
    switch (parameter->element_type) {
        case ODIN_ELEMENT_TYPE_UINT8:
            return snprintf(output_buffer, output_buffer_size, "%" PRIu16, (uint16_t) * (uint8_t *)data);

        case ODIN_ELEMENT_TYPE_INT8:
            return snprintf(output_buffer, output_buffer_size, "%" PRId16, (int16_t) * (int8_t *)data);

        case ODIN_ELEMENT_TYPE_UINT16:
            return snprintf(output_buffer, output_buffer_size, "%" PRIu16, *(uint16_t *)data);

        case ODIN_ELEMENT_TYPE_INT16:
            return snprintf(output_buffer, output_buffer_size, "%" PRId16, *(int16_t *)data);

        case ODIN_ELEMENT_TYPE_UINT32:
            return snprintf(output_buffer, output_buffer_size, "%" PRIu32, *(uint32_t *)data);

        case ODIN_ELEMENT_TYPE_INT32:
            return snprintf(output_buffer, output_buffer_size, "%" PRId32, *(int32_t *)data);

        case ODIN_ELEMENT_TYPE_UINT64:
            return snprintf(output_buffer, output_buffer_size, "%" PRIu64, *(uint64_t *)data);

        case ODIN_ELEMENT_TYPE_INT64:
            return snprintf(output_buffer, output_buffer_size, "%" PRId64, *(int64_t *)data);

        case ODIN_ELEMENT_TYPE_BOOL:
            return snprintf(output_buffer, output_buffer_size, "%s", (*(bool *)data) ? "true" : "false");

        case ODIN_ELEMENT_TYPE_FLOAT32:
            return snprintf(output_buffer, output_buffer_size, "%f", (double)*(float *)data);

        case ODIN_ELEMENT_TYPE_FLOAT64:
            return snprintf(output_buffer, output_buffer_size, "%f", *(double *)data);

        case ODIN_ELEMENT_TYPE_HEX:
            return snprintf(output_buffer, output_buffer_size, "0x%02x", *(uint8_t *)data);

        case ODIN_ELEMENT_TYPE_CHAR:
            return snprintf(output_buffer, output_buffer_size, "%c", *(char *)data);

        case ODIN_ELEMENT_TYPE_CUSTOM: {
            // Check if the parameter has a custom string serialisation extension
            const ODIN_extension_t *extension = find_extension(parameter, ODIN_EXTENSION_TYPE_STRING_CODEC);
            if (extension == NULL) {
                return ODIN_ERROR_NOT_SUPPORTED;
            }

            ODIN_string_serialisation_extension_ops_t *ops = (ODIN_string_serialisation_extension_ops_t *)extension->ops;
            return ops->to_string(parameter, data, parameter->element_size, output_buffer, output_buffer_size);
        }
        default:
            return ODIN_ERROR_NOT_SUPPORTED;
    }
}

// TODO: Add tests for this function
int ODIN_string_to_element(const ODIN_parameter_t *parameter, const char *input_buffer, void *data, size_t data_size) {
    switch (parameter->element_type) {
        case ODIN_ELEMENT_TYPE_UINT8:
            return sscanf(input_buffer, "%" SCNu8, (uint8_t *)data);

        case ODIN_ELEMENT_TYPE_INT8:
            return sscanf(input_buffer, "%" SCNd8, (int8_t *)data);

        case ODIN_ELEMENT_TYPE_UINT16:
            return sscanf(input_buffer, "%" SCNu16, (uint16_t *)data);

        case ODIN_ELEMENT_TYPE_INT16:
            return sscanf(input_buffer, "%" SCNd16, (int16_t *)data);

        case ODIN_ELEMENT_TYPE_UINT32:
            return sscanf(input_buffer, "%" SCNu32, (uint32_t *)data);

        case ODIN_ELEMENT_TYPE_INT32:
            return sscanf(input_buffer, "%" SCNd32, (int32_t *)data);

        case ODIN_ELEMENT_TYPE_UINT64:
            return sscanf(input_buffer, "%" SCNu64, (uint64_t *)data);

        case ODIN_ELEMENT_TYPE_INT64:
            return sscanf(input_buffer, "%" SCNd64, (int64_t *)data);
        case ODIN_ELEMENT_TYPE_FLOAT32:
            return sscanf(input_buffer, "%f", (float *)data);

        case ODIN_ELEMENT_TYPE_FLOAT64:
            return sscanf(input_buffer, "%lf", (double *)data);

            // TODO: add support to the other types
            //  case ODIN_ELEMENT_TYPE_BOOL: {
            //      bool value;
            //      int ret = sscanf(input_buffer, "%d", &value);
            //      if (ret == 1) {
            //          *(bool *)data = value;
            //          return 1;
            //      }
            //      return ret;
            //  }

            // case ODIN_ELEMENT_TYPE_HEX:
            //     return sscanf(input_buffer, "0x%02x", (uint8_t *)data);

            // case ODIN_ELEMENT_TYPE_CUSTOM: {
            //     // Check if the parameter has a custom string serialisation extension
            //     const ODIN_extension_t *extension = find_extension(parameter, ODIN_EXTENSION_TYPE_STRING_CODEC);
            //     if (extension == NULL) {
            //         return ODIN_ERROR_NOT_SUPPORTED;
            //     }

            //     ODIN_string_serialisation_extension_ops_t *ops = (ODIN_string_serialisation_extension_ops_t *)extension->ops;
            //     return ops->from_string(parameter, input_buffer, data, data_size);
            // }

        default:
            return ODIN_ERROR_NOT_SUPPORTED;
    }
}

/**
 * @brief Serialise a parameter to a human readable string
 *
 * @param parameter Pointer to the parameter
 * @param output_buffer Pointer to the buffer to store the string
 * @param size Size of the buffer
 * @param access_group Access group to check if the parameter is accessible
 *
 * @return int Length of the string
 */
int ODIN_encode_parameter_to_string(const ODIN_parameter_t *parameter, char *output_buffer, size_t output_buffer_size,
                                    odin_access_group_t access_group) {
    // TODO: Check if the parameter is accessible

    uint8_t data_buffer[ODIN_PRINT_BUFFER_SIZE];

    int ret_size = ODIN_parameter_read_into_buffer(parameter, data_buffer, sizeof(data_buffer), access_group);
                                       
    if (ret_size < 0) {
        return ret_size;
    }

    if (ret_size == 0) {
        return snprintf(output_buffer, output_buffer_size, "No data");
    }

    switch (parameter->odin_type) {
        case ODIN_TYPE_PARAMETER:
            return ODIN_element_to_string(parameter, data_buffer, output_buffer, output_buffer_size);

        case ODIN_TYPE_VECTOR:
        case ODIN_TYPE_ARRAY: {
            // In case of a string, print it as a string
            if (parameter->element_type == ODIN_ELEMENT_TYPE_CHAR) {
                // make sure the string is null terminated
                if (ret_size >= output_buffer_size) {
                    ret_size = output_buffer_size - 1;
                }
                data_buffer[ret_size] = '\0';
                return snprintf(output_buffer, output_buffer_size, "\"%s\"", (char *)data_buffer);
            }

            // Print all elements formatted in square brackets
            int len = 0;
            output_buffer[len++] = '[';
            uint8_t *data_buffer_ptr = data_buffer;

            for (; data_buffer_ptr < data_buffer + ret_size; data_buffer_ptr += parameter->element_size) {
                int ret = ODIN_element_to_string(parameter, data_buffer_ptr, output_buffer + len, output_buffer_size - len);
                if (ret < 0) {
                    return ret;
                }

                len += ret;

                // if not last
                if (data_buffer_ptr < data_buffer + ret_size - parameter->element_size) {
                    output_buffer[len++] = ',';
                    output_buffer[len++] = ' ';
                }

                // if buffer is, nearly full add ... and break
                if (len >= output_buffer_size - 10) {
                    len += snprintf(output_buffer + len, output_buffer_size - len, "...");
                    break;
                }
            }
            output_buffer[len++] = ']';
            output_buffer[len] = '\0';

            return len;
        } break;

        default:
            return ODIN_ERROR_NOT_SUPPORTED;
    }
}
int ODIN_decode_string_to_parameter(const ODIN_parameter_t *parameter, const char *input_buffer,
                                    odin_access_group_t access_group) {
    // Only support writing to parameters for now, not vectors and arrays
    if (parameter->odin_type != ODIN_TYPE_PARAMETER) {
        return ODIN_ERROR_NOT_SUPPORTED;
    }

    // Decode the string to the parameter
    uint8_t data_buffer[ODIN_PRINT_BUFFER_SIZE];
    int ret = ODIN_string_to_element(parameter, input_buffer, data_buffer, sizeof(data_buffer));

    if (ret < 0) {
        return ret;
    }

    // Write the data to the parameter
    return ODIN_parameter_write(parameter, data_buffer, parameter->element_size, access_group);
}
