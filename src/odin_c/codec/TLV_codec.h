
#include "odin_core.h"
#include "odin_security.h"


int ODIN_encode_parameter_to_bytes(const ODIN_parameter_t *parameter, uint8_t *output_buffer, size_t output_buffer_size,
                                   odin_access_group_t access_group);

int ODIN_decode_bytes_to_parameter(const ODIN_parameter_group_t *group, const uint8_t *input_buffer, size_t input_buffer_size,
                                   odin_access_group_t access_group);

int ODIN_encode_parameter_group_to_bytes(const ODIN_parameter_group_t *group, uint8_t *output_buffer, size_t output_buffer_size,
                                         odin_access_group_t access_group);

int ODIN_decode_bytes_to_parameter_group(const ODIN_parameter_group_t *group, const uint8_t *input_buffer,
                                         size_t input_buffer_size, odin_access_group_t access_group);
