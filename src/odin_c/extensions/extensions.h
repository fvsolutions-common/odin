#include <stddef.h>

#ifndef EXTENSIONS_H
#define EXTENSIONS_H
// #include "odin_security.h"

enum odin_acces_group;

struct ODIN_parameter;

// String serialisation extension type
typedef struct {
    // Represents data as a string
    int (*to_string)(const struct ODIN_parameter *parameter, const void *data, size_t size, char *buffer, size_t buffer_size);

    // Parses string into data
    int (*from_string)(const struct ODIN_parameter *parameter, const char *string, void *data, size_t size);
} ODIN_string_serialisation_extension_ops_t;

// IO extension type
typedef struct {
    // Write data to the parameter
    int (*write)(const struct ODIN_parameter *parameter, const void *data, size_t size, enum odin_acces_group access_group);

    // Read data from the parameter
    int (*read)(const struct ODIN_parameter *parameter, void *data, size_t size, enum odin_acces_group access_group);
} ODIN_io_extension_ops_t;

typedef struct {
    // Validates the data
    int (*validate)(const struct ODIN_parameter *parameter, const void *data, size_t size, enum odin_acces_group access_group);

} ODIN_validate_extension_ops_t;

typedef enum {
    ODIN_EXTENSION_TYPE_IO,
    ODIN_EXTENSION_TYPE_STRING_CODEC,
    ODIN_EXTENSION_TYPE_VALIDATE,
} ODIN_extension_type_t;

// Extension definition
struct ODIN_extension;

typedef struct ODIN_extension {
    ODIN_extension_type_t type;   // Type of the extension
    void *ops;                    // Operations for the extension, check the type to cast to the
                                  // correct type
    void *data;                   // Metadata for the extension, can be NULL
    struct ODIN_extension *next;  // Next extension in the list, can be NULL
} ODIN_extension_t;

extern ODIN_validate_extension_ops_t ODIN_validate_extension_ops;
extern ODIN_io_extension_ops_t ODIN_extension_io_mapped_number_ops;
const ODIN_extension_t *find_extension(const struct ODIN_parameter *parameter, ODIN_extension_type_t type);

typedef struct {
    float min;
    float max;
} range_parameter_t;

typedef struct {
    const struct ODIN_parameter *reference;
    float scale;
    float offset;
} mapped_number_parameters_t;



#endif