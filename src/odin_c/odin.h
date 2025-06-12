#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "extensions/extensions.h"
#ifndef ODIN_H
#define ODIN_H

struct ODIN_extension;

// Temp references
extern uint64_t current_time_in_microseconds;
struct my_struct {
    float reference;
};

extern struct my_struct my;


typedef enum {
    ODIN_SUCCESS = 0,
    ODIN_ERROR = -9,
    ODIN_ERROR_NO_PARAMETER = -10,
    ODIN_ERROR_INVALID_ARGUMENT = -11,
    ODIN_ERROR_PARAMETER_NOT_FOUND = -12,
    ODIN_ERROR_SIZE_MISMATCH = -13,
    ODIN_ERROR_BUFFER_TOO_SMALL = -14,
    ODIN_ERROR_PERMISSION_DENIED = -15,
    ODIN_ERROR_UNSUPPORTED_FORMAT = -16,
    ODIN_ERROR_NOT_SUPPORTED = -17,
    ODIN_ERROR_FILE_NOT_FOUND = -18,
    ODIN_ERROR_INVALID_PARAMETER_ACTION = -19,
    ODIN_ERROR_VALIDATION = -19
} odin_error_t;

typedef enum {
    ODIN_TYPE_PARAMETER,  // Single value
    ODIN_TYPE_ARRAY,      // Fixed length array
    ODIN_TYPE_VECTOR,     // Preallocated variable length array (keeps track of the
                          // current length, with a separate pointer)
    ODIN_TYPE_GROUP       // Group of parameters
} ODIN_type_t;

typedef enum {
    ODIN_ELEMENT_TYPE_BOOL,

    ODIN_ELEMENT_TYPE_HEX,  // Same UINT8, but will be displayed as hex

    ODIN_ELEMENT_TYPE_UINT8,
    ODIN_ELEMENT_TYPE_UINT16,
    ODIN_ELEMENT_TYPE_UINT32,
    ODIN_ELEMENT_TYPE_UINT64,

    ODIN_ELEMENT_TYPE_INT8,
    ODIN_ELEMENT_TYPE_INT16,
    ODIN_ELEMENT_TYPE_INT32,
    ODIN_ELEMENT_TYPE_INT64,

    ODIN_ELEMENT_TYPE_FLOAT32,
    ODIN_ELEMENT_TYPE_FLOAT64,

    ODIN_ELEMENT_TYPE_CHAR,
    ODIN_ELEMENT_TYPE_CUSTOM  // Custom data type, custom to string and validator
                              // functions have to be provided
} ODIN_element_type_t;
 
struct ODIN_parameter;

#define ODIN_INCLUDE_NAME
#define ODIN_INCLUDE_DESCRIPTION

typedef struct {
    size_t num_elements;
    uint8_t data[2];  // Not a pointer, but a array
} ODIN_vector_structure_t;

#pragma pack(push, 1)
typedef struct ODIN_parameter {
    // Local ID of the parameter
    uint32_t global_index;

    // Type of the parameter
    ODIN_type_t odin_type : 8;

    // Type of the element
    ODIN_element_type_t element_type : 8;

    // Extra flags
    uint16_t flags;

    // Size of a single element
    uint16_t element_size;

    // Max elements for arrays and vectors, otherwise 1
    uint16_t max_elements;

    // Pointer to the data
    void *data;

    // Name of the parameter, double null terminated, the first null is the end of
    // the string, the second null is the end of description
#ifdef ODIN_INCLUDE_NAME
    const char *name_and_description;
#endif

    // Extension info
    const struct ODIN_extension *extension;  // Linked list of extensions, can be NULL

} ODIN_parameter_t;
#pragma pack(pop)

typedef struct {
    uint32_t global_index;  // Bits to shift to get the index
    ODIN_type_t odin_type : 4;
} ODIN_parameter_generic_t;

typedef struct {
    uint32_t global_index;  // Bits to shift to get the index
    ODIN_type_t odin_type : 4;
    uint16_t shift;  // Bits to shift to get the index
    uint16_t type;   // Number of parameters in the group
    uint16_t count;  // Number of parameters in the group

    const char *name_and_description;
    const void *parameters[];
} ODIN_parameter_group_t;

// A return eatly macro if not successful, which only executes the code once
#define RETURN_ON_FAIL(x) \
    do {                  \
        int ret = (x);    \
        if (ret < 0) {    \
            return ret;   \
        }                 \
    } while (0)

#endif  // ODIN_H