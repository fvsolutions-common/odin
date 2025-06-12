from odin_python.parameter.loader import ConfigurationReader
from odin_python.generators.generator import GeneratorTarget, generator
from pathlib import Path

import pytest

POSTITIVE_TEST_CONFIGS = list(Path("test/test_configs").glob("*.yaml"))
NEGATIVE_TEST_CONFIGS = list(Path("test/flawed_test_configs").glob("*.yaml"))


def process_config(config_file: Path):
    model_context, config_model = ConfigurationReader().load(config_file.as_posix(), "advanced")

    output_dir = Path("test/demo") / config_file.stem

    output_dir.mkdir(parents=True, exist_ok=True)

    for target in GeneratorTarget.all():
        print(f" - creating target '{target.name}'")
        generator(
            name="OD",
            model_context=model_context,
            output_dir=output_dir.as_posix(),
            target=target,
            generator_config=config_model,
        )


@pytest.mark.parametrize("config", POSTITIVE_TEST_CONFIGS, ids=[str(config.name) for config in POSTITIVE_TEST_CONFIGS])
def test_all_configs(config: Path):
    print(f"processing config: '{config}")
    process_config(config)


@pytest.mark.parametrize("config", NEGATIVE_TEST_CONFIGS, ids=[str(config.name) for config in NEGATIVE_TEST_CONFIGS])
def test_all_flawed_configs(config: Path):
    print(f"processing config: '{config}")
    with pytest.raises(Exception):
        process_config(config)
