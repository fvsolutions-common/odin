cmake_minimum_required(VERSION 3.15...3.26)

# If ESP-IDF is reading this CMakeLists.txt, register it as an ESP-IDF component
if(DEFINED ENV{IDF_PATH})
  idf_component_register(SRCS
      src/odin_c/odin_core.c
      src/odin_c/codec/TLV_codec.c
      src/odin_c/codec/protobuff_codec.c
      src/odin_c/codec/string_codec.c
      src/odin_c/codec/number_codec.c
      src/odin_c/codec/json_codec.c
      src/odin_c/extensions/validate_extension.c
      src/odin_c/extensions/io_extension.c
      src/odin_c/extensions/extensions.c
      src/odin_c/odin_utils.c
      src/odin_c/odin_security.c
      src/odin_c/odin_lookup.c
      INCLUDE_DIRS src/odin_c
      REQUIRES "main")
    return()
endif()

# If Zephyr is reading this CMakeLists.txt, register it as a Zephyr module
if(Zephyr_FOUND)
  zephyr_library_named(odin)
  zephyr_library_sources(
        src/odin_c/odin_core.c
        src/odin_c/codec/TLV_codec.c
        src/odin_c/codec/string_codec.c
        src/odin_c/codec/number_codec.c
        # src/odin_c/codec/protobuff_codec.c  #Currently not supported in zephyr, needs a bit more work
        # src/odin_c/codec/json_codec.c  #Currently not supported in zephyr, needs a bit more work
        src/odin_c/extensions/validate_extension.c
        src/odin_c/extensions/io_extension.c
        src/odin_c/extensions/extensions.c
        src/odin_c/odin_utils.c
        src/odin_c/odin_security.c
        src/odin_c/odin_lookup.c
  )
  target_include_directories(odin PUBLIC src/odin_c)

  return()
endif()

# TODO Fix the llvm based build
error("This CMakeLists.txt was not able to dettect your build system.")

# Otherwhise compile with LLVM
cmake_minimum_required(VERSION 3.10)
project(ODIN VERSION 1.0 LANGUAGES C CXX)

# Set the C and C++ compilers
set(CMAKE_C_COMPILER "C:/Program Files/LLVM/bin/clang.exe")
set(CMAKE_CXX_COMPILER "C:/Program Files/LLVM/bin/clang++.exe")

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set default build type to Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# export compile_commands.json for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add coverage flags
if(CMAKE_BUILD_TYPE STREQUAL "Coverage")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-instr-generate -fcoverage-mapping")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-instr-generate -fcoverage-mapping")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-instr-generate -fcoverage-mapping")
endif()


# Add nanopb

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../nanopb/extra)
find_package(Nanopb REQUIRED)


add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(examples)