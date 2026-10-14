"""Vendor tool installs on the host, resolved into read-only container mounts. Nothing from a tool is
copied anywhere; a missing install resolves to None and the caller reports the step as not runnable."""
from __future__ import annotations

import os
from dataclasses import dataclass, field
from pathlib import Path

DEFAULT_MGC_HOME = "/tools/software/siemens/catapult/latest/Mgc_home"
DEFAULT_MGC_LICENSE = "1717@ece-winlic.ece.gatech.edu"
DEFAULT_VITIS_HLS = "/tools/software/xilinx/ARCHIVE/Vitis_HLS/2024.1"
DEFAULT_XILINX_LICENSE = "2100@ece-winlic.ece.gatech.edu"
DEFAULT_BAMBU_APPDIR = "/export/hdd/scratch/tshukla31/tools/bambu-2024.10"


@dataclass(frozen=True)
class ToolInstallation:
    volumes: dict[str, dict[str, str]] = field(default_factory=dict)  # host path -> docker volume spec
    environment: dict[str, str] = field(default_factory=dict)


def _ro(host: Path, container: str) -> dict[str, dict[str, str]]:
    return {str(host): {"bind": container, "mode": "ro"}}


def catapult_installation(mgc_home: str | Path | None = None) -> ToolInstallation | None:
    root = Path(mgc_home or os.environ.get("MGC_HOME", DEFAULT_MGC_HOME)).resolve()
    if not (root / "bin" / "catapult").is_file():
        return None
    lic = os.environ.get("SALT_LICENSE_FILE") or os.environ.get("MGLS_LICENSE_FILE") or DEFAULT_MGC_LICENSE
    return ToolInstallation(
        _ro(root, "/opt/siemens/catapult"),
        {"MGC_HOME": "/opt/siemens/catapult", "MGLS_LICENSE_FILE": lic, "SALT_LICENSE_FILE": lic},
    )


def vitis_installation(vitis_dir: str | Path | None = None, vivado_dir: str | Path | None = None) -> ToolInstallation | None:
    """Legacy layout `<root>/Vitis_HLS/<version>` with the matching `<root>/Vivado/<version>` (csynth needs its
    part database); the layout of the lab install. After hls-eval's resolver."""
    hls = Path(vitis_dir or os.environ.get("XILINX_HLS", DEFAULT_VITIS_HLS)).resolve()
    if not (hls / "bin" / "vitis_hls").is_file():
        return None
    vivado = Path(vivado_dir).resolve() if vivado_dir else hls.parent.parent / "Vivado" / hls.name
    if not (vivado / "bin" / "vivado").is_file() or not (vivado / "data" / "parts").is_dir():
        return None
    lic = os.environ.get("XILINXD_LICENSE_FILE") or DEFAULT_XILINX_LICENSE
    return ToolInstallation(
        {**_ro(hls, "/opt/amd/hls"), **_ro(vivado, "/opt/amd/vivado")},
        {"XILINX_HLS": "/opt/amd/hls", "XILINX_VIVADO": "/opt/amd/vivado", "XILINXD_LICENSE_FILE": lic,
         "PATH": "/opt/amd/hls/bin:/usr/local/bin:/usr/bin:/bin"},
    )


def bambu_installation(app_dir: str | Path | None = None) -> ToolInstallation | None:
    """An extracted Bambu AppImage (`bambu-<ver>.AppImage --appimage-extract`): AppRun puts the bundled
    clang, PandA data and soft libm on the path, so the driver calls `$BAMBU` rather than `bambu`."""
    root = Path(app_dir or os.environ.get("BAMBU_APPDIR", DEFAULT_BAMBU_APPDIR)).resolve()
    if not (root / "AppRun").is_file() or not (root / "usr" / "bin" / "bambu").is_file():
        return None
    return ToolInstallation(
        _ro(root, "/opt/panda/bambu"),
        {"APPDIR": "/opt/panda/bambu", "ARGV0": "bambu", "OWD": "/work", "BAMBU": "/opt/panda/bambu/AppRun"},
    )


RESOLVERS = {"catapult": catapult_installation, "vitis": vitis_installation, "bambu": bambu_installation}


def installation(tool: str) -> ToolInstallation | None:
    return RESOLVERS[tool]() if tool in RESOLVERS else None
