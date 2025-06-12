#include "odin.h"
#include "odin_security.h"

int ODIN_parameter_read(const ODIN_parameter_t *parameter, void *data, size_t size, odin_access_group_t access_group);
int ODIN_parameter_write(const ODIN_parameter_t *parameter, const void *data, size_t size,
                         odin_access_group_t access_group);

int ODIN_parameter_read_into_buffer(const ODIN_parameter_t *parameter, void *output_buffer, size_t output_buffer_size,
                                    odin_access_group_t access_group);

int ODIN_get_data_size(const ODIN_parameter_t *parameter);
int ODIN_get_max_data_size(const ODIN_parameter_t *parameter);
int ODIN_array_read_element(const ODIN_parameter_t *parameter, int index, void *data, size_t size,
                            odin_access_group_t access_group);
int ODIN_array_write_element(const ODIN_parameter_t *parameter, int index, const void *data, size_t size,
                             odin_access_group_t access_group);