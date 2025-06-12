#include "odin_core.h"

#include "extensions/extensions.h"
#include "string.h"

#define TAG "ODIN"

/**
 * @brief Method to read a parameter
 *
 * @param parameter Pointer to the parameter
 * @param data Pointer to the data buffer
 * @param size Size of the data to read, needs to be exactly the size of the
 * parameter in case of a parameter or array
 * @param access_group The access group, which is requesting the read
 *
 * @return if negative an error code is returned, otherwise the size of the data
 * read in bytes
 */
int ODIN_parameter_read(const ODIN_parameter_t *parameter, void *data, size_t size, odin_access_group_t access_group) {
    // Check if the parameter exists
    if (parameter == NULL || data == NULL) {
        return ODIN_ERROR_INVALID_ARGUMENT;
    }

    // Check if the permission is allowed
    RETURN_ON_FAIL(ODIN_validate_access(parameter, access_group, ODIN_ACCESS_READ));

    // // Handle validation
    // const ODIN_extension_t *extension = find_extension(parameter, ODIN_EXTENSION_TYPE_STRING_CODEC);
    // if (extension != NULL) {
    //     ODIN_io_extension_ops_t *ops = (ODIN_io_extension_ops_t *)extension->ops;
    //     int ret = ops->read(parameter, data, size, access_group);
    //     if (ret < ODIN_SUCCESS) {
    //         printf( "Extension read failed, error %d ,parameter %s", ret, parameter->name_and_description);
    //         return ret;
    //     }
    // }

    // Handle the io extension
    const ODIN_extension_t *extension = find_extension(parameter, ODIN_EXTENSION_TYPE_IO);
    if (extension != NULL) {
        ODIN_io_extension_ops_t *ops = (ODIN_io_extension_ops_t *)extension->ops;
        return ops->read(parameter, data, size, access_group);
    }

    // If the parameter has no data, return an error
    if (parameter->data == NULL) {
        return ODIN_ERROR_INVALID_PARAMETER_ACTION;
    }

    // Otherwise, copy the data to the buffer
    int read_size = ODIN_get_data_size(parameter);

    // Check if the size matches the read size
    if (parameter->odin_type == ODIN_TYPE_VECTOR) {
        if (size < read_size) {
            printf("Read size to small (%zu < %u) when reading %s", size, read_size, parameter->name_and_description);
            return ODIN_ERROR_SIZE_MISMATCH;
        }

        ODIN_vector_structure_t *vector_structure = (ODIN_vector_structure_t *)parameter->data;

        // Copy the data
        memcpy(data, vector_structure->data, read_size);

    } else {
        if (size != read_size) {
            printf("Read size mismatch (%zu != %u) when reading %s", size, parameter->element_size,
                   parameter->name_and_description);
            return ODIN_ERROR_SIZE_MISMATCH;
        }

        memcpy(data, parameter->data, read_size);
    }

    // Log the access
    // ODIN_conditionally_log_access(parameter, access_group, ODIN_READ_ACCESS);

    return read_size;
}

int ODIN_parameter_read_into_buffer(const ODIN_parameter_t *parameter, void *output_buffer, size_t output_buffer_size,
                                    odin_access_group_t access_group) {
    int total_size = ODIN_get_data_size(parameter);

    // Check if the buffer is large enough
    if (output_buffer_size < total_size) {
        return ODIN_ERROR_BUFFER_TOO_SMALL;
    }
    return ODIN_parameter_read(parameter, output_buffer, total_size, access_group);
}

int ODIN_get_data_size(const ODIN_parameter_t *parameter) {
    if (parameter->odin_type == ODIN_TYPE_VECTOR) {
        uint16_t *num_elements = (uint16_t *)parameter->data;
        if (*num_elements > parameter->max_elements) {
            return parameter->element_size * parameter->max_elements;
        } else {
            return parameter->element_size * *num_elements;
        }
    } else if (parameter->odin_type == ODIN_TYPE_ARRAY) {
        return parameter->element_size * parameter->max_elements;
    } else {
        return parameter->element_size;
    }
}
/**
 * @brief Method to get the maximum data size of a parameter
 *
 * @param parameter Pointer to the parameter
 *
 * @return The maximum data size in bytes
 */
int ODIN_get_max_data_size(const ODIN_parameter_t *parameter) {
    switch (parameter->odin_type) {
        case ODIN_TYPE_VECTOR:
        case ODIN_TYPE_ARRAY:
            return parameter->element_size * parameter->max_elements;

        case ODIN_TYPE_PARAMETER:
            return parameter->element_size;

        case ODIN_TYPE_GROUP:
            return 0;
    }
    return 0;
}

/**
 * @brief Method to write a parameter
 *
 * @param parameter Pointer to the parameter
 * @param data Pointer to the data buffer
 * @param size Size of the data to write, needs to be exactly the size of the
 * parameter in case of a parameter or array
 * @param access_group The access group, which is requesting the write
 *
 * @return if negative an error code is returned, otherwise the size of the data
 * written in bytes
 */
int ODIN_parameter_write(const ODIN_parameter_t *parameter, const void *data, size_t size, odin_access_group_t access_group) {
    // Check if all the parameters are valid
    if (parameter == NULL || data == NULL) {
        return ODIN_ERROR_INVALID_ARGUMENT;
    }

    // Check if the permission is allowed
    RETURN_ON_FAIL(ODIN_validate_access(parameter, access_group, ODIN_ACCESS_WRITE));

    // Handle validation
    const ODIN_extension_t *extension = find_extension(parameter, ODIN_EXTENSION_TYPE_VALIDATE);
    if (extension != NULL) {
        ODIN_validate_extension_ops_t *ops = (ODIN_validate_extension_ops_t *)extension->ops;
        int ret = ops->validate(parameter, data, size, access_group);
        if (ret < ODIN_SUCCESS) {
            printf("Extension validate failed, error %d ,parameter %s", ret, parameter->name_and_description);
            return ret;
        }
    }

    // Handle the io extension
    extension = find_extension(parameter, ODIN_EXTENSION_TYPE_IO);
    if (extension != NULL) {
        ODIN_io_extension_ops_t *ops = (ODIN_io_extension_ops_t *)extension->ops;
        return ops->write(parameter, data, size, access_group);
    }

    // If the parameter has no data, return an error
    if (parameter->data == NULL) {
        return ODIN_ERROR_INVALID_PARAMETER_ACTION;
    }

    // Otherwise, copy the data to the buffer
    int max_write_size = ODIN_get_max_data_size(parameter);

    // Check if the size matches the write size
    if (parameter->odin_type == ODIN_TYPE_VECTOR) {
        if (size > max_write_size) {
            printf("Write size to large (%zu > %u) when writing %s", size, max_write_size, parameter->name_and_description);
            return ODIN_ERROR_SIZE_MISMATCH;
        }

        // Check if a non fractional number of elements is written
        if (size % parameter->element_size != 0) {
            printf("Write size not a multiple of element size (%zu %% %u != 0) when writing %s", size, parameter->element_size,
                   parameter->name_and_description);
            return ODIN_ERROR_SIZE_MISMATCH;
        }

        ODIN_vector_structure_t *vector_structure = (ODIN_vector_structure_t *)parameter->data;

        // Copy the data
        memcpy(vector_structure->data, data, size);

        // Update the element count
        vector_structure->num_elements = size / parameter->element_size;

    } else {
        if (size != max_write_size) {
            printf("Write size mismatch (%zu != %u) when writing %s", size, max_write_size, parameter->name_and_description);
            return ODIN_ERROR_SIZE_MISMATCH;
        }

        memcpy(parameter->data, data, size);
    }

    // Log the access
    // ODIN_conditionally_log_access(parameter, access_group, ODIN_WRITE_ACCESS);

    return size;
}

/**
 * @brief Method to write an element of an array
 *
 * @param parameter Pointer to the parameter
 * @param index Index of the element to write
 * @param data Pointer to the data buffer
 * @param size Size of the data to write
 * @param access_group The access group, which is requesting the write
 *
 * @return if negative an error code is returned, otherwise the size of the data
 * written in bytes
 */

int ODIN_array_write_element(const ODIN_parameter_t *parameter, int index, const void *data, size_t size,
                             odin_access_group_t access_group) {
    // Check if all the parameters are valid
    if (parameter == NULL || data == NULL) {
        return ODIN_ERROR_INVALID_ARGUMENT;
    }

    // Check if type is array
    if (parameter->odin_type != ODIN_TYPE_ARRAY) {
        printf("Invalid type for array write %d", parameter->odin_type);
        return ODIN_ERROR_INVALID_ARGUMENT;
    }

    // Check if the permission is allowed
    RETURN_ON_FAIL(ODIN_validate_access(parameter, access_group, ODIN_ACCESS_WRITE));

    // Handle the extension (TODO)
    // if (parameter->extension != NULL && parameter->extension->ops->write !=
    // NULL)
    // {
    //     int ret = parameter->extension->ops->write(parameter, data, size,
    //     access_group); if (ret)
    //     {
    //         printf( "Extension write failed, error %d ,parameter %s", ret,
    //         parameter->name);
    //     }
    //     return ret;
    // }

    // Otherwise, copy the data to the buffer
    // int max_write_size = ODIN_get_max_data_size(parameter);

    // Check if the size matches the write size
    if (index >= parameter->max_elements) {
        printf("Write index out of bounds (%d >= %u) when writing %s", index, parameter->max_elements,
               parameter->name_and_description);
        return ODIN_ERROR_SIZE_MISMATCH;
    }

    // Check if the size matches the write size
    if (size != parameter->element_size) {
        printf("Write size mismatch (%zu != %u) when writing %s", size, parameter->element_size, parameter->name_and_description);
        return ODIN_ERROR_SIZE_MISMATCH;
    }

    // Calculate the offset
    int offset = index * parameter->element_size;

    // Copy the data
    memcpy((uint8_t *)parameter->data + offset, data, size);

    // Log the access
    // ODIN_conditionally_log_access(parameter, access_group, ODIN_WRITE_ACCESS);

    return size;
}

/**
 * @brief Method to read an element of an array
*
* @param parameter Pointer to the parameter
* @param index Index of the element to read
* @param data Pointer to the data buffer
* @param size Size of the data to read
* @param access_group The access group, which is requesting the read
*
* @return if negative an error code is returned, otherwise the size of the data
read in bytes
*/
int ODIN_array_read_element(const ODIN_parameter_t *parameter, int index, void *data, size_t size,
                            odin_access_group_t access_group) {
    // Check if all the parameters are valid
    if (parameter == NULL || data == NULL) {
        return ODIN_ERROR_INVALID_ARGUMENT;
    }

    // Check if type is array
    if (parameter->odin_type != ODIN_TYPE_ARRAY) {
        printf("Invalid type for array read %d", parameter->odin_type);
        return ODIN_ERROR_INVALID_ARGUMENT;
    }

    // Check if the permission is allowed
    RETURN_ON_FAIL(ODIN_validate_access(parameter, access_group, ODIN_ACCESS_READ));

    // Handle the extension (TODO)
    // if (parameter->extension != NULL && parameter->extension->ops->read !=
    // NULL)
    // {
    //     int ret = parameter->extension->ops->read(parameter, data, size,
    //     access_group); if (ret)
    //     {
    //         printf( "Extension read failed, error %d ,parameter %s", ret,
    //         parameter->name);
    //     }
    //     return ret;
    // }

    // Otherwise, copy the data to the buffer
    // int max_read_size = ODIN_get_max_data_size(parameter);

    // Check if the size matches the read size
    if (index >= parameter->max_elements) {
        printf("Read index out of bounds (%d >= %u) when reading %s", index, parameter->max_elements,
               parameter->name_and_description);
        return ODIN_ERROR_SIZE_MISMATCH;
    }

    // Check if the size matches the read size
    if (size != parameter->element_size) {
        printf("Read size mismatch (%zu != %u) when reading %s", size, parameter->element_size, parameter->name_and_description);
        return ODIN_ERROR_SIZE_MISMATCH;
    }

    // Calculate the offset
    int offset = index * parameter->element_size;

    // Copy the data
    memcpy(data, (uint8_t *)parameter->data + offset, size);

    // Log the access
    // ODIN_conditionally_log_access

    return size;
}
