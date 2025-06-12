from csnake import CodeWriterLite
import csnake as cc
from typing import Union


class StructVariable(cc.Struct):
    """Custom implementation of a struct to allow recursive struct generation"""

    def add_struct(self, struct: cc.Struct):
        self.variables.append(struct)  # type: ignore

    def __init__(self, name: str = "", typedef: bool = False, var_name: str = ""):
        self.var_name = var_name
        super().__init__(name, typedef=typedef)

    def generate_declaration(self, indent: Union[int, str] = 4) -> CodeWriterLite:
        writer = CodeWriterLite(indent=indent)

        if self.typedef:
            writer.add_line("typedef struct")
        else:
            writer.add_line(f"struct {self.name}")

        writer.open_brace()

        for var in self.variables:
            if isinstance(var, cc.Struct):
                writer.add_lines(var.generate_declaration(indent=4).code)  # type: ignore
            else:
                writer.add_line(var.declaration)

        writer.close_brace()

        if self.typedef:
            writer.add(" " + self.name + ";")
        else:
            writer.add(f"{self.var_name};")

        return writer

    def add_variable(self, variable: cc.Variable):  # type: ignore
        super().add_variable(variable)  # type: ignore
