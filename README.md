# ODIN

ODIN is platform agnostic object dictionary for embedded systems, it consists of parts:
1) **A python code generation tool**, which parsed the yaml definition file and generates C code and other artifacts.
2) **A C library**, which provides the runtime for the object dictionary, it is written in C and can be used in C (and C++ projects). **This runtime only depends on the C standard** library and can be used in any embedded platform.


![definition](docs/definition.svg)


## ODIN-Python

### Installation

This project depends on UV, which is a python package manager, installation instructions can be found here: https://docs.astral.sh/uv/guides/install-python/

If this is in installed an availavle in your PATH, you can access the code generation as follows:

```bash
uv run odin [ARGS...] 
```
### Usage

To create the object dictionary, you need to create a yaml file, which describes the object dictionary. You can find an examples in the `test/test_configs` directory. 

Once you have created the yaml file, you can run the code generation tool:

```bash
uv run odin generate [CONFIG_PATH] [OUTPUT_DIR]
```

This will generate the all the files it can generate, including the C code, python library, and documentation. You can also specify the generation options, use the --help option to see the available options:

```bash
uv run odin generate --help
```

#### Schema
The schema for validating the yaml file can be generated using the following command:

```bash
uv run odin gen-schema schema.json
```


## ODIN-C
This contains the runtime library for the object dictionary, it is written in C and can be integrated in C (and C++) projects. It provides the following features:
* Lookup by name and index
* Getting and setting values using a generic interface
    * With support for Arrays and vectors
* String serialization and deserialization to allow interaction from for example a CLI
* Access control mechanism to allow for read-only and write-only access to objects depending on the access rights
* Multiple codes to allow for different serialization formats, such as JSON, TVL, protobuf, to allow for the data do be serialised to for example a file or sent over a network

See `docs/DOCS.md` for more information about the design and usage of the object dictionary.

### Integration
To integrate the ODIN C library into your project, you can use the CMakelist.txt, this currently support the following frameworks.
* Zephyr
* ESP-IDF

using a makefile (only) based build system, just compile the nessecary files and include the header files in your project.

# Testing
The ODIN library is tested using the `pytest` framework, you can run the tests using the following command:

```bash
uv run pytest test
```

ODIN-has 91% test coverage, you can check the coverage report using the following command:

```bash
uv run pytest --cov=odin_python --cov-report=term test
```

Or use the code [coverage feature](https://code.visualstudio.com/docs/python/testing#_run-tests-with-coverage) in vscode



# Open Source
This project is open source and licensed under the MPL-2.0 license, see the LICENSE file for more information.

This allows you to use the ODIN library in proprietary codebases, as longs as this library itself and modifications to it are made accessible under the same license.

