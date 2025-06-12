
#include "codec/odin.pb.h"
#include "odin_core.h"

#ifndef ODIN_CODEC_PROTOBUFF_CODEC_H
#define ODIN_CODEC_PROTOBUFF_CODEC_H

typedef struct {
    odin_access_group_t access_group;
    const ODIN_parameter_t *parameter;
    odin_proto_parameter proto;
} parameter_encode_operation_t;

typedef struct {
    const ODIN_parameter_group_t *parameter_group;
    odin_access_group_t access_group;
    odin_proto_parameter_collection proto;
} parameter_group_encode_operation_t;

typedef struct {
    odin_access_group_t access_group;
    const ODIN_parameter_group_t *parameter_group;
} parameter_collection_decode_operation_t;

typedef struct {
    odin_access_group_t access_group;
    odin_proto_parameter *proto;
    const ODIN_parameter_group_t *parameter_group;
} parameter_decode_operation_t;

// Encoding of a signle_protobuff parameter
odin_error_t ODIN_encode_parameter_to_protobuff_init(const ODIN_parameter_t *parameter, parameter_encode_operation_t *operation,
                                                     odin_access_group_t access_group);
int ODIN_encode_parameter_to_protobuff_buffer(parameter_encode_operation_t *operation, uint8_t *buffer, size_t size);
odin_error_t ODIN_encode_parameter_to_protobuff_stream(parameter_encode_operation_t *operation, pb_ostream_t *stream);

// Encoding of a parameter group to protobuff
odin_error_t ODIN_encode_parameter_group_to_protobuff_init(const ODIN_parameter_group_t *parameter_group,
                                                           parameter_group_encode_operation_t *operation, uint16_t access_group);
int ODIN_encode_parameter_group_to_protobuff_buffer(parameter_group_encode_operation_t *operation, uint8_t *buffer, size_t size);
odin_error_t ODIN_encode_parameter_group_to_protobuff_stream(parameter_group_encode_operation_t *operation, pb_ostream_t *stream);
odin_error_t ODIN_encode_parameter_group_to_protobuff_file(const ODIN_parameter_group_t *parameter_group, const char *filename,
                                                            odin_access_group_t access_group);
// Decoding of a parameter group from protobuff
odin_error_t ODIN_decode_protobuff_buffer_to_parameter_group(const ODIN_parameter_group_t *parameter_group, uint8_t *buffer,
                                                               size_t size, odin_access_group_t access_group);

odin_error_t ODIN_decode_protobuff_stream_to_parameter_group(const ODIN_parameter_group_t *parameter_group,
                                                               pb_istream_t *stream, odin_access_group_t access_group);

int ODIN_decode_protobuff_file_to_parameter_group(const ODIN_parameter_group_t *parameter_group, const char *filename,
                                                    odin_access_group_t access_group);
#endif  // ODIN_CODEC_PROTOBUFF_CODEC_H