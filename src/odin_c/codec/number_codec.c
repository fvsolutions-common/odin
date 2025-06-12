#include "number_codec.h"

#include "odin.h"

/**
 * @brief Method to read a parameter as a floating point number
 *
 * @param parameter Pointer to the parameter
 * @param data Pointer to the data buffer
 * @param size Size of the data buffer
 * @param access_group Access group
 *
 * @return int error code
 */
int ODIN_encode_parameter_to_float(const ODIN_parameter_t *parameter, float *data, odin_access_group_t access_group) {
    if (parameter == NULL || data == NULL) {
        return ODIN_ERROR_INVALID_ARGUMENT;
    }

    // Only parameters can be read as floats, not arrays or vectors
    if (parameter->odin_type != ODIN_TYPE_PARAMETER) {
        return ODIN_ERROR_INVALID_ARGUMENT;
    }

    return ODIN_encode_data_to_float(parameter->element_type, parameter->data, parameter->element_size, data);
}

int ODIN_encode_data_to_float(ODIN_element_type_t element_type, const uint8_t *data, size_t size, float *value) {
    if (data == NULL) {
        return ODIN_ERROR_INVALID_ARGUMENT;
    }

    // TODO validate size of data

    switch (element_type) {
        case ODIN_ELEMENT_TYPE_FLOAT32:
            (*value) = *((float *)data);
            break;

        case ODIN_ELEMENT_TYPE_FLOAT64:
            (*value) = *((double *)data);
            break;

        case ODIN_ELEMENT_TYPE_INT8:
            (*value) = (float)(*((int8_t *)data));
            break;

        case ODIN_ELEMENT_TYPE_UINT8:
            (*value) = (float)(*((uint8_t *)data));
            break;

        case ODIN_ELEMENT_TYPE_INT16:
            (*value) = (float)(*((int16_t *)data));
            break;

        case ODIN_ELEMENT_TYPE_UINT16:
            (*value) = (float)(*((uint16_t *)data));
            break;

        case ODIN_ELEMENT_TYPE_INT32:
            (*value) = (float)(*((int32_t *)data));
            return sizeof(int32_t);

        case ODIN_ELEMENT_TYPE_UINT32:
            (*value) = (float)(*((uint32_t *)data));
            return sizeof(uint32_t);

        case ODIN_ELEMENT_TYPE_BOOL:
            (*value) = (float)(*((bool *)data));
            break;

        default:
            return ODIN_ERROR;
    }

    return ODIN_SUCCESS;
}
int ODIN_decode_float_to_data(ODIN_element_type_t element_type, const uint8_t *data, size_t size, float value) {
    if (data == NULL) {
        return ODIN_ERROR_INVALID_ARGUMENT;
    }

    // TODO validate size of data

    switch (element_type) {
        case ODIN_ELEMENT_TYPE_FLOAT32:
            *((float *)data) = value;
            break;

        case ODIN_ELEMENT_TYPE_FLOAT64:
            *((double *)data) = value;
            break;

        case ODIN_ELEMENT_TYPE_INT8:
            *((int8_t *)data) = (int8_t)value;
            break;

        case ODIN_ELEMENT_TYPE_UINT8:
            *((uint8_t *)data) = (uint8_t)value;
            break;

        case ODIN_ELEMENT_TYPE_INT16:
            *((int16_t *)data) = (int16_t)value;
            break;

        case ODIN_ELEMENT_TYPE_UINT16:
            *((uint16_t *)data) = (uint16_t)value;
            break;

        case ODIN_ELEMENT_TYPE_INT32:
            *((int32_t *)data) = (int32_t)value;
            break;

        case ODIN_ELEMENT_TYPE_UINT32:
            *((uint32_t *)data) = (uint32_t)value;
            break;

        case ODIN_ELEMENT_TYPE_BOOL:
            *((bool *)data) = value;
            break;

        default:
            return ODIN_ERROR;
    }
    return ODIN_SUCCESS;
}

int ODIN_decode_float_to_parameter(const ODIN_parameter_t *parameter, float data, odin_access_group_t access_group) {
    if (parameter == NULL) {
        return ODIN_ERROR_INVALID_ARGUMENT;
    }

    if (parameter->odin_type != ODIN_TYPE_PARAMETER) {
        return ODIN_ERROR_INVALID_ARGUMENT;
    }

    return ODIN_decode_float_to_data(parameter->element_type, parameter->data, parameter->element_size, data);
}
