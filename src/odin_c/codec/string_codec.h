
#include "odin_core.h"
#include "odin_security.h"

int ODIN_element_to_string(const ODIN_parameter_t *parameter, void *data, char *output_buffer, size_t output_buffer_size);

int ODIN_encode_parameter_to_string(const ODIN_parameter_t *parameter, char *output_buffer, size_t output_buffer_size,
                                    odin_access_group_t access_group);

int ODIN_decode_string_to_parameter(const ODIN_parameter_t *parameter, const char *input_buffer,
                                    odin_access_group_t access_group);
