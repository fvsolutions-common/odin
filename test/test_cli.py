from pathlib import Path
from click.testing import CliRunner
from odin_python.cli import cli


def test_schema_generation():
    runner = CliRunner()
    result = runner.invoke(cli, ["gen-schema", "test/demo/schema.json"])
    assert result.exit_code == 0, f"CLI command failed with error: {result.output}"



def test_invalid_schema_path():
    runner = CliRunner()
    result = runner.invoke(cli, ["gen-schema", "nonexistent/schema.json"])
    assert result.exit_code != 0, "CLI command should fail with invalid schema path"


def test_invalid_config_path():
    runner = CliRunner()
    result = runner.invoke(cli, ["generate", "nonexistent.yaml", "test/demo"])
    assert result.exit_code != 0, "CLI command should fail with invalid config path"


def test_invalid_config_invalid_file():
    runner = CliRunner()
    result = runner.invoke(cli, ["generate", "README.md", "test/demo"])
    assert result.exit_code != 0, "CLI command should fail with invalid config file"


def test_generate_with_invalid_folder():
    runner = CliRunner()
    result = runner.invoke(cli, ["generate", "test/test_configs/config.yaml", "test/nonexistent"])
    assert result.exit_code != 0, "CLI command should fail with invalid target"


def test_generate_with_invalid_target():
    runner = CliRunner()
    result = runner.invoke(cli, ["generate", "test/test_configs/config.yaml", "test/demo", "--target", "invalid"])
    assert result.exit_code != 0, "CLI command should fail with invalid target"


def test_generate_with_valid_target():
    runner = CliRunner()
    result = runner.invoke(cli, ["generate", "test/test_configs/config.yaml", "test/demo"])
    assert result.exit_code == 0, f"CLI command failed with error: {result.output}"
