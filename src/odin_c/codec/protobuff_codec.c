#include "protobuff_codec.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "inttypes.h"
#include "odin.h"
#include "odin.pb.h"
#include "odin_codec.h"
#include "odin_core.h"
#include "odin_lookup.h"
#include "pb_decode.h"
#include "pb_encode.h"

static bool encode_parameter_callback(pb_ostream_t *stream, const pb_field_t *field, void *const *arg);
static bool encode_collection_callback(pb_ostream_t *stream, const pb_field_t *field, void *const *arg);
static bool decode_parameter_callback(pb_istream_t *stream, const pb_field_t *field, void **arg);
static bool decode_collection_callback(pb_istream_t *stream, const pb_field_t *field, void **arg);
static bool _file_write(pb_ostream_t *stream, const pb_byte_t *buf, size_t count);
static bool _file_read(pb_istream_t *stream, pb_byte_t *buf, size_t count);

static const char *TAG = "ODIN_STR";

#define ODIN_OPERATION_BUFFER_SIZE 128

/**
 * @brief Initialises the encode operation
 * @param parameter The parameter to encode
 * @param operation The operation to initialise, this will be used as context for the encoding
 * @param access_group The access group, which is responsible for this request
 */
odin_error_t ODIN_encode_parameter_to_protobuff_init(const ODIN_parameter_t *parameter, parameter_encode_operation_t *operation,
                                                     odin_access_group_t access_group) {
    if (parameter == NULL) {
        return ODIN_ERROR_INVALID_ARGUMENT;
    }

    operation->parameter = parameter;
    operation->access_group = access_group;
    operation->proto.id = parameter->global_index;
    operation->proto.data.funcs.encode = encode_parameter_callback;
    operation->proto.data.arg = operation;

    return ODIN_SUCCESS;
}

/**
 * @brief Encodes a single parameter to a byte buffer
 * Used as a callback function for the protobuf encoder, to do the actual encoding
 */
static bool encode_parameter_callback(pb_ostream_t *stream, const pb_field_t *field, void *const *arg) {
    parameter_encode_operation_t *operation = (parameter_encode_operation_t *)*arg;

    if (operation == NULL || operation->parameter == NULL) {
        return false;
    }

    // Add the tag
    if (!pb_encode_tag_for_field(stream, field)) {
        return false;
    }

    uint8_t data[ODIN_OPERATION_BUFFER_SIZE];
    int size = ODIN_parameter_read_into_buffer(operation->parameter, data, sizeof(data), operation->access_group);

    if (size < 0) {
        return false;
    }

    return pb_encode_string(stream, data, size);
}

/**
 * @brief Encodes a single parameter to a protobuf stream
 * Used as a callback function for the protobuf encoder, to do the actual encoding
 *
 * @param operation The encode operation, pre-initialised with the parameter to encode
 * @param stream The stream to write the data to
 *
 * @return
 *  - ODIN_ERROR if the encoding failed
 *  - ODIN_SUCCESS if the encoding was successful
 */
odin_error_t ODIN_encode_parameter_to_protobuff_stream(parameter_encode_operation_t *operation, pb_ostream_t *stream) {
    int ret = pb_encode(stream, odin_proto_parameter_fields, &operation->proto);

    if (!ret) {
        return ODIN_ERROR;
    }

    return ODIN_SUCCESS;
}

/**
 * @brief export a parameter to a buffer in RAM
 *
 * @param operation The encode operation, pre-initialised with the parameter to encode
 * @param buffer The buffer to write the data to
 * @param size The size of the buffer
 *
 * @return
 *  - ODIN_ERROR if the encoding failed
 *  - or the number of bytes that have been written in the buffer
 */
int ODIN_encode_parameter_to_protobuff_buffer(parameter_encode_operation_t *operation, uint8_t *buffer, size_t size) {
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, size);
    int ret = pb_encode(&stream, odin_proto_parameter_fields, &operation->proto);

    if (!ret) {
        return ODIN_ERROR;
    }

    return stream.bytes_written;
}

/**
 * @brief Initialises the encode operation
 * @param parameter_group The parameter group to encode
 * @param operation The operation to initialise, this will be used as context for the encoding
 * @param access_group The access group, which is responsible for this request
 */
odin_error_t ODIN_encode_parameter_group_to_protobuff_init(const ODIN_parameter_group_t *parameter_group,
                                                           parameter_group_encode_operation_t *operation, uint16_t access_group) {
    if (parameter_group == NULL || operation == NULL) {
        return ODIN_ERROR_INVALID_ARGUMENT;
    }
    operation->parameter_group = parameter_group;
    operation->access_group = access_group;
    operation->proto.parameters.funcs.encode = encode_collection_callback;
    operation->proto.parameters.arg = operation;
    return ODIN_SUCCESS;
}
/**
 * @brief Callback function to encode a collection of parameters into a protobuf stream.
 *
 * This function recursively encodes a collection of parameters, which can include nested parameter groups,
 * into a protobuf stream.
 *
 * @param stream The protobuf output stream to encode the parameters into.
 * @param field The protobuf field descriptor for the current field being encoded.
 * @param arg A pointer to the parameter group encode operation structure.
 *
 * @return true if the encoding was successful, false otherwise.
 */

static bool encode_collection_callback(pb_ostream_t *stream, const pb_field_t *field, void *const *arg) {
    parameter_group_encode_operation_t *operation = (parameter_group_encode_operation_t *)*arg;

    for (size_t index = 0; index < operation->parameter_group->count; index++) {
        const ODIN_parameter_generic_t *generic_parameter = operation->parameter_group->parameters[index];

        // This should never happen
        if (generic_parameter == NULL) {
            return false;
        }

        if (generic_parameter->odin_type == ODIN_TYPE_GROUP) {
            const ODIN_parameter_group_t *group = (const ODIN_parameter_group_t *)generic_parameter;

            // Recursively encode the group
            parameter_group_encode_operation_t group_operation = {
                .parameter_group = group,
                .access_group = operation->access_group,
            };

            void *data = &group_operation;

            if (!encode_collection_callback(stream, field, &data)) {
                return false;
            }

        } else {
            const ODIN_parameter_t *parameter = (const ODIN_parameter_t *)generic_parameter;

            // Add the proto parameter to the stream
            if (!pb_encode_tag_for_field(stream, field)) {
                return false;
            }

            // Create the encode parameter operation
            parameter_encode_operation_t param_operation;
            if (ODIN_encode_parameter_to_protobuff_init(parameter, &param_operation, operation->access_group) != ODIN_SUCCESS) {
                return false;
            }

            // Encode the parameter
            if (!pb_encode_submessage(stream, odin_proto_parameter_fields, &param_operation.proto)) {
                return false;
            }
        }
    }

    return true;
}

/**
 * @brief export a group to a buffer in RAM
 *
 * @return
 *  - ODIN_ERROR
 *  - or the number of bytes that have been written in the buffer
 */
int ODIN_encode_parameter_group_to_protobuff_buffer(parameter_group_encode_operation_t *operation, uint8_t *buffer, size_t size) {
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, size);
    int ret = pb_encode(&stream, odin_proto_parameter_collection_fields, &(operation->proto));

    if (!ret) {
        return ODIN_ERROR;
    }

    return stream.bytes_written;
}

odin_error_t ODIN_encode_parameter_group_to_protobuff_stream(parameter_group_encode_operation_t *operation,
                                                             pb_ostream_t *stream) {
    int ret = pb_encode(stream, odin_proto_parameter_collection_fields, &operation->proto);

    if (!ret) {
        return ODIN_ERROR;
    }

    return ODIN_SUCCESS;
}

odin_error_t ODIN_encode_parameter_group_to_protobuff_file(const ODIN_parameter_group_t *parameter_group, const char *filename,
                                                              odin_access_group_t access_group) {
    FILE *file = fopen( filename, "wb");
    if (file == NULL) {
        printf( "Failed to open file %s", filename);
        return ODIN_ERROR_FILE_NOT_FOUND;
    }

    // Determine the file size

    pb_ostream_t stream = {
        .callback = _file_write,
        .state = file,
        .max_size = SIZE_MAX,
    };

    // Write the collection
    parameter_group_encode_operation_t operation;
    RETURN_ON_FAIL(ODIN_encode_parameter_group_to_protobuff_init(parameter_group, &operation, access_group));

    // Encode the collection
    int ret = ODIN_encode_parameter_group_to_protobuff_stream(&operation, &stream);
    if (ret != ODIN_SUCCESS) {
        printf( "Failed to encode parameter collection");
        return ret;
    }

    fclose(file);
    return ODIN_SUCCESS;
}

static bool _file_write(pb_ostream_t *stream, const pb_byte_t *buf, size_t count) {
    FILE *file = (FILE *)stream->state;
    size_t written = fwrite(buf, 1, count, file);
    return written == count;
}

// DECODING
/**
 * @brief Decodes a Protocol Buffers buffer into a parameter group.
 *
 * This function takes a buffer containing Protocol Buffers encoded data and decodes it
 * into the provided parameter group structure.
 *
 * @param parameter_group Pointer to the parameter group structure where the decoded data will be stored.
 * @param buffer Pointer to the buffer containing the Protocol Buffers encoded data.
 * @param size Size of the buffer in bytes.
 * @param access_group Access group used to control access to the parameters.
 * @return Returns an odin_error_t indicating the success or failure of the decoding operation.
 */

odin_error_t ODIN_decode_protobuff_buffer_to_parameter_group(const ODIN_parameter_group_t *parameter_group, uint8_t *buffer,
                                                               size_t size, odin_access_group_t access_group) {
    pb_istream_t stream = pb_istream_from_buffer(buffer, size);
    return ODIN_decode_protobuff_stream_to_parameter_group(parameter_group, &stream, access_group);
}

/**
 * @brief Decodes a protobuf stream into a parameter collection from a given parameter group.
 *
 * This function takes a protobuf input stream and decodes it into a parameter collection
 * based on the provided parameter group and access group.
 *
 * @param parameter_group Pointer to the parameter group used for decoding.
 * @param stream Pointer to the protobuf input stream to be decoded.
 * @param access_group Access group used for decoding.
 * @return ODIN_SUCCESS if decoding is successful, otherwise ODIN_ERROR.
 */

odin_error_t ODIN_decode_protobuff_stream_to_parameter_group(const ODIN_parameter_group_t *parameter_group,
                                                               pb_istream_t *stream, odin_access_group_t access_group) {
    parameter_collection_decode_operation_t operation = {
        .access_group = access_group,
        .parameter_group = parameter_group,
    };

    odin_proto_parameter_collection decoded = odin_proto_parameter_collection_init_zero;
    decoded.parameters.funcs.decode = decode_collection_callback;
    decoded.parameters.arg = &operation;

    // Decode the collection
    bool ret = pb_decode(stream, odin_proto_parameter_collection_fields, &decoded);

    if (!ret) {
        printf( "Failed to decode parameter collection");
        return ODIN_ERROR;
    }

    return ODIN_SUCCESS;
}

int ODIN_decode_protobuff_file_to_parameter_group(const ODIN_parameter_group_t *parameter_group, const char *filename,
                                                    odin_access_group_t access_group) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        printf( "Failed to open file %s", filename);
        return ODIN_ERROR_FILE_NOT_FOUND;
    }
    
    // Determine the file size
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    pb_istream_t stream = {
        .callback = _file_read,
        .state = file,
        .bytes_left = size,
    };

    // Create a stream
    int ret = ODIN_decode_protobuff_stream_to_parameter_group(parameter_group, &stream, access_group);
    fclose(file);

    if (ret != ODIN_SUCCESS) {
        printf( "Failed to import parameters from file");
        return ODIN_ERROR;
    }

    return ODIN_SUCCESS;
}

static bool _file_read(pb_istream_t *stream, pb_byte_t *buf, size_t count) {
    FILE *file = (FILE *)stream->state;
    size_t read = fread(buf, 1, count, file);
    return read == count;
}

/**
 * @brief Callback function to decode a collection of parameters from a protobuf stream.
 *
 * This function is called during the decoding process of a protobuf stream to decode
 * a collection of parameters.
 *
 * @param stream Pointer to the protobuf input stream.
 * @param field Pointer to the protobuf field descriptor.
 * @param arg Pointer to the argument passed to the callback, which is expected to be
 *            a pointer to a parameter_collection_decode_operation_t structure.
 * @return true if decoding is successful, false otherwise.
 */
static bool decode_collection_callback(pb_istream_t *stream, const pb_field_t *field, void **arg) {
    parameter_collection_decode_operation_t *operation = (parameter_collection_decode_operation_t *)*arg;

    // Decode all parameters
    odin_proto_parameter proto_parameter = odin_proto_parameter_init_zero;
    proto_parameter.data.funcs.decode = decode_parameter_callback;

    parameter_decode_operation_t param_operation = {
        .access_group = operation->access_group,
        .parameter_group = operation->parameter_group,
        .proto = &proto_parameter,
    };

    proto_parameter.data.arg = &param_operation;

    // Decode all parameters
    while (pb_decode(stream, odin_proto_parameter_fields, &proto_parameter)) {
    }

    return true;
}

/**
 * @brief Callback function to decode a parameter from a protobuf stream.
 *
 * This function reads data from the provided protobuf input stream, finds the corresponding
 * parameter using the parameter ID, and writes the decoded data to the parameter.
 *
 * @param stream Pointer to the protobuf input stream.
 * @param field Pointer to the protobuf field descriptor.
 * @param arg Pointer to the user-defined argument, which is expected to be a pointer to a
 *            parameter_decode_operation_t structure.
 * @return true if the parameter was successfully decoded and written, false otherwise.
 */
static bool decode_parameter_callback(pb_istream_t *stream, const pb_field_t *field, void **arg) {
    parameter_decode_operation_t *operation = (parameter_decode_operation_t *)*arg;

    // read the data
    uint8_t buffer[128];
    size_t size = stream->bytes_left;

    if (!pb_read(stream, buffer, size)) {
        printf( "decode: Failed to read parameter data");
        return false;
    }

    // Find the parameter
    const ODIN_parameter_t *parameter = ODIN_get_parameter_by_id(operation->parameter_group, operation->proto->id, 0);
    if (parameter == NULL) {
        printf( "decode: Failed to find parameter with id %ld", operation->proto->id);
        return false;
    }

    // Set the data
    if (ODIN_parameter_write(parameter, buffer, size, operation->access_group) < ODIN_SUCCESS) {
        printf( "decode: Failed to set parameter");
        return false;
    }

    return true;
}

// static bool _file_read(pb_istream_t *stream, pb_byte_t *buf, size_t count) {
//     FILE *file = (FILE *)stream->state;
//     size_t read = fread(buf, 1, count, file);
//     return read == count;
// }

// int ODIN_protobuff_import_from_file(const ODIN_parameter_group_t *store, const char *filename, uint16_t access_group) {
//     FILE *file = fopen(filename, "rb");
//     if (file == NULL) {
//         printf( "Failed to open file %s", filename);
//         return ODIN_ERR_FILE_NOT_FOUND;
//     }

//     // Determine the file size
//     fseek(file, 0, SEEK_END);
//     long size = ftell(file);
//     fseek(file, 0, SEEK_SET);

//     pb_istream_t stream = {
//         .callback = _file_read,
//         .state = file,
//         .bytes_left = size,
//     };

//     // Create a stream
//     int ret = ODIN_protobuff_import_params_from_stream(&stream, store, access_group);
//     fclose(file);

//     if (ret != ODIN_SUCCESS) {
//         printf( "Failed to import parameters from file");
//         return ODIN_ERROR;
//     }

//     return ODIN_SUCCESS;
// }

// static bool _file_write(pb_ostream_t *stream, const pb_byte_t *buf, size_t count) {
//     FILE *file = (FILE *)stream->state;
//     size_t written = fwrite(buf, 1, count, file);
//     return written == count;
// }

// int ODIN_protobuff_export_group_to_file(const ODIN_parameter_group_t *group, const char *filename, uint16_t access_group) {
//     FILE *file = fopen(filename, "wb");
//     if (file == NULL) {
//         printf( "Failed to open file %s", filename);
//         return ODIN_ERROR;
//     }

//     pb_ostream_t stream = {
//         .callback = _file_write,
//         .state = file,
//         .max_size = SIZE_MAX,
//     };

//     // Write the collection
//     parameter_collection_encode_operation_t operation;
//     const ODIN_parameter_t *parameter_buffer[128];
//     ODIN_protobuff_encode_collection_init(parameter_buffer, 128, false, &operation, access_group);
//     ODIN_add_group_to_collection(&operation, group);

//     // Encode the collection
//     int ret = ODIN_protobuff_export_collection_to_stream(&operation, &stream);
//     if (ret != ODIN_SUCCESS) {
//         printf( "Failed to encode parameter collection");
//         return ret;
//     }

//     fclose(file);

//     return ODIN_SUCCESS;
// }

// int ODIN_protobuff_export_group_to_buffer(const ODIN_parameter_group_t *group, uint8_t *buffer, size_t *size,
//                                           uint16_t access_group) {
//     // Write the collection
//     parameter_collection_encode_operation_t operation;
//     const ODIN_parameter_t *parameter_buffer[128];
//     ODIN_protobuff_encode_collection_init(parameter_buffer, 128, false, &operation, access_group);
//     ODIN_add_group_to_collection(&operation, group);

//     // Encode the collection
//     int ret = ODIN_protobuff_export_collection_to_buffer(&operation, buffer, *size);
//     if (ret < 0) {
//         return ret;
//     }
//     *size = ret;

//     return ODIN_SUCCESS;
// }

// int ODIN_protobuff_export_store_to_file(const ODIN_parameter_group_t *store, const char *filename, uint16_t access_group) {
//     FILE *file = fopen(filename, "wb");
//     if (file == NULL) {
//         printf( "Failed to open file %s", filename);
//         return ODIN_ERROR;
//     }

//     pb_ostream_t stream = {
//         .callback = _file_write,
//         .state = file,
//         .max_size = SIZE_MAX,
//     };

//     // Write the collection
//     parameter_collection_encode_operation_t operation;
//     const ODIN_parameter_t *parameter_buffer[128];
//     ODIN_protobuff_encode_collection_init(parameter_buffer, 128, false, &operation, access_group);
//     ODIN_add_store_to_collection(&operation, store);

//     // Encode the collection
//     int ret = ODIN_protobuff_export_collection_to_stream(&operation, &stream);
//     if (ret != ODIN_SUCCESS) {
//         printf( "Failed to encode parameter collection");
//         return ret;
//     }

//     fclose(file);

//     return ODIN_SUCCESS;
// }
