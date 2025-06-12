add_library(odin STATIC
    odin_core.c
    codec/TLV_codec.c
    codec/protobuff_codec.c
    codec/string_codec.c
    codec/number_codec.c
    extensions/validate_extension.c
    extensions/io_extension.c
    extensions/extensions.c
    odin_utils.c
    odin_security.c
    odin_lookup.c
)

# Expose the src directory as the include path for public headers
target_include_directories(odin PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)


nanopb_generate_cpp(TARGET proto codec/pb/odin.proto)

target_link_libraries(odin proto)