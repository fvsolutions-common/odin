#include "odin_security.h"

int ODIN_encode_parameter_to_float(const ODIN_parameter_t *parameter, float *data, odin_access_group_t access_group);
int ODIN_decode_float_to_parameter(const ODIN_parameter_t *group, float data, odin_access_group_t access_group);
int ODIN_encode_data_to_float(ODIN_element_type_t element_type, const uint8_t *data, size_t size, float *value);
int ODIN_decode_float_to_data(ODIN_element_type_t element_type, const uint8_t *data, size_t size, float value);
