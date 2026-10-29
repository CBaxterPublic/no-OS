"""Produce or locate no-OS boot artifacts for HIL tests.

Build mode runs tools/scripts/no_os_build.py, whose output directory holds both
the .elf and BOOT.BIN.
"""

from __future__ import annotations

import dataclasses
import subprocess
import sys
import zipfile
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[3]
NO_OS_BUILD = REPO_ROOT / "tools" / "scripts" / "no_os_build.py"

# Artifacts field -> glob pattern; first sorted match wins.
_ARTIFACT_PATTERNS = {
    "elf": "*.elf",
    "boot_bin": "BOOT.BIN",
    "hex": "*.hex",
    "uf2": "*.uf2",
    "bin": "*.bin",
    "xsa": "*.xsa",
}


@dataclasses.dataclass
class Artifacts:
    build_dir: Path
    elf: Path | None = None
    boot_bin: Path | None = None
    hex: Path | None = None
    uf2: Path | None = None
    bin: Path | None = None
    xsa: Path | None = None

    def require(self, field: str) -> Path:
        value = getattr(self, field, None)
        if value is None:
            raise FileNotFoundError(f"no {field} artifact found under {self.build_dir}")
        return value


def discover_artifacts(root) -> Artifacts:
    """Recursively find boot artifacts under root; first match per type wins."""
    root = Path(root)
    found = {}
    for field, pattern in _ARTIFACT_PATTERNS.items():
        matches = sorted(root.rglob(pattern))
        if matches:
            found[field] = matches[0]
    return Artifacts(build_dir=root, **found)


def unpack_if_zip(path, dest) -> Path:
    """If path is a .zip, extract into dest and return dest; else return path.

    Assumes a trusted archive (team-controlled CI artifacts); does not sanitize
    member paths beyond zipfile's built-in traversal protection.
    """
    path = Path(path)
    if path.suffix == ".zip":
        dest = Path(dest)
        dest.mkdir(parents=True, exist_ok=True)
        with zipfile.ZipFile(path) as z:
            z.extractall(dest)
        return dest
    return path


def build_via_no_os_build(*, project, variant, board, builds_dir,
                          python=sys.executable, hardware=None) -> Path:
    """Run no_os_build.py for one project/variant/board. Returns the artifacts dir.

    The build lands in <builds_dir>/<project>-<variant>-<board>/build, which
    holds the .elf and (for xilinx) BOOT.BIN. Raises
    subprocess.CalledProcessError on build failure (check=True).
    """
    builds_dir = Path(builds_dir)
    argv = [
        python, str(NO_OS_BUILD), "build",
        "--project", project, "--variant", variant, "--board", board,
        "--build-dir", str(builds_dir),
    ]
    if hardware:
        argv += ["--hardware", str(hardware)]
    subprocess.run(argv, cwd=REPO_ROOT, check=True)
    return builds_dir / f"{project}-{variant}-{board}" / "build"
