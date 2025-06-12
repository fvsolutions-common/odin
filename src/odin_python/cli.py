import json
import os

import click

from .generators.generator import generator, GeneratorTarget
from .parameter.loader import AdvancedLoaderModel, ConfigurationReader

DEFAULT_NAME = "OD"


@click.group()
def cli():
    pass


@cli.command()
@click.argument("input_file")
@click.argument("output_dir")
@click.option("name", "--name", default=DEFAULT_NAME, help="Name of the output files")
@click.option(
    "--target",
    multiple=True,
    type=click.Choice([target.name for target in GeneratorTarget.all()]),
    help="Target language for code generation",
)
def generate(input_file: str, output_dir: str, name: str, target: list[str]):
    assert os.path.exists(input_file), f"Input file {input_file} does not exist"
    assert os.path.exists(output_dir), f"Output directory {output_dir} does not exist"

    # Check if the input file is a valid yaml file
    assert input_file.endswith(".yaml"), "Input file must be a yaml file"

    print(f"Converting {input_file} to {output_dir}")

    reader = ConfigurationReader()
    model_context, config_model = reader.load(input_file, "advanced")

    resolved_targets = [GeneratorTarget.from_string(t) for t in target]

    if len(resolved_targets) == 0:
        resolved_targets = GeneratorTarget.all()

    for resolved_target in resolved_targets:
        generator(
            name=name,
            model_context=model_context,
            output_dir=output_dir,
            target=resolved_target,
            generator_config=config_model,
        )


@cli.command()
@click.argument("output_file")
def gen_schema(output_file: str, simple: bool = False):
    with open(output_file, "w") as f:
        f.write(json.dumps(AdvancedLoaderModel.model_json_schema(), indent=4))


if __name__ == "__main__":
    cli()
