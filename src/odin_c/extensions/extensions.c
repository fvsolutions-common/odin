#include "extensions.h"
#include "odin.h"

const ODIN_extension_t *find_extension(const struct ODIN_parameter *parameter, ODIN_extension_type_t type) {
    const ODIN_extension_t *extension = parameter->extension;

    while (extension != NULL) {
        if (extension->type == type) {
            return extension;
        }
        extension = extension->next;
    }

    return NULL;
}