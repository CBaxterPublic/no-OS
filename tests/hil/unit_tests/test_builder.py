import zipfile

import pytest

import builder


def test_discover_artifacts_first_match(tmp_path):
    (tmp_path / "sub").mkdir()
    (tmp_path / "app.elf").write_bytes(b"elf")
    (tmp_path / "sub" / "BOOT.BIN").write_bytes(b"boot")
    arts = builder.discover_artifacts(tmp_path)
    assert arts.elf == tmp_path / "app.elf"
    assert arts.boot_bin == tmp_path / "sub" / "BOOT.BIN"
    assert arts.hex is None
    assert arts.build_dir == tmp_path


def test_require_raises_when_missing(tmp_path):
    arts = builder.discover_artifacts(tmp_path)
    with pytest.raises(FileNotFoundError):
        arts.require("elf")


def test_unpack_if_zip(tmp_path):
    z = tmp_path / "a.zip"
    with zipfile.ZipFile(z, "w") as zf:
        zf.writestr("x.elf", "e")
    dest = tmp_path / "out"
    out = builder.unpack_if_zip(z, dest)
    assert out == dest
    assert (dest / "x.elf").exists()


def test_unpack_if_zip_passthrough_for_dir(tmp_path):
    assert builder.unpack_if_zip(tmp_path, tmp_path / "unused") == tmp_path


def test_require_unknown_field_raises_filenotfound(tmp_path):
    arts = builder.discover_artifacts(tmp_path)
    with pytest.raises(FileNotFoundError):
        arts.require("not_a_field")


def test_build_via_no_os_build_argv(tmp_path, monkeypatch):
    calls = {}

    def fake_run(argv, cwd=None, check=None):
        calls["argv"] = argv
        calls["cwd"] = cwd
        calls["check"] = check
        return None

    monkeypatch.setattr(builder.subprocess, "run", fake_run)
    out = builder.build_via_no_os_build(
        project="adrv9009", variant="demo", board="zc706",
        builds_dir=tmp_path, python="python3", hardware="/x/system_top.xsa")
    assert out == tmp_path / "adrv9009-demo-zc706" / "build"
    argv = calls["argv"]
    assert argv[:3] == ["python3", str(builder.NO_OS_BUILD), "build"]
    assert argv[argv.index("--project") + 1] == "adrv9009"
    assert argv[argv.index("--variant") + 1] == "demo"
    assert argv[argv.index("--board") + 1] == "zc706"
    assert argv[argv.index("--build-dir") + 1] == str(tmp_path)
    assert argv[argv.index("--hardware") + 1] == "/x/system_top.xsa"
    assert calls["check"] is True


def test_build_via_no_os_build_omits_empty_hardware(tmp_path, monkeypatch):
    calls = {}
    monkeypatch.setattr(builder.subprocess, "run",
                        lambda argv, cwd=None, check=None: calls.setdefault("argv", argv))
    builder.build_via_no_os_build(
        project="ad9361", variant="iio", board="zed", builds_dir=tmp_path)
    assert "--hardware" not in calls["argv"]
