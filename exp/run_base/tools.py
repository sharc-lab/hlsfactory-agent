from dataclasses import dataclass
from pathlib import Path
import shutil
import subprocess
import time
from typing import Any

import psutil


def auto_find_vitis_hls_bin() -> Path | None:
    vitis_hls_bin_path_str = shutil.which("vitis_hls")
    if vitis_hls_bin_path_str is None:
        return None
    vitis_hls_bin = Path(vitis_hls_bin_path_str).resolve()
    if not vitis_hls_bin.exists():
        raise RuntimeError(f"vitis_hls found in PATH but missing: {vitis_hls_bin}")
    return vitis_hls_bin


@dataclass
class ExecutionData:
    return_code: int
    stdout: str
    stderr: str
    t0: float
    t1: float
    execution_time: float
    timeout: bool

    def to_dict(self) -> dict[str, Any]:
        return {
            "return_code": self.return_code,
            "stdout": self.stdout,
            "stderr": self.stderr,
            "t0": self.t0,
            "t1": self.t1,
            "execution_time": self.execution_time,
            "timeout": self.timeout,
        }


@dataclass
class ToolDataOutput:
    data_execution: ExecutionData
    data_tool: None | dict


class VitisHLSSynthTool:
    def __init__(self, vitis_hls_bin: Path | None = None) -> None:
        if vitis_hls_bin is None:
            vitis_hls_bin = auto_find_vitis_hls_bin()
        if vitis_hls_bin is None:
            raise RuntimeError("Unable to find `vitis_hls` in PATH.")
        self.vitis_hls_bin = vitis_hls_bin.resolve()

    @staticmethod
    def _terminate_process_tree(pid: int) -> None:
        process = psutil.Process(pid=pid)
        children = process.children(recursive=True)
        for child in children:
            child.terminate()
        process.terminate()

    def run(
        self,
        tcl_script_fp: Path,
        work_dir: Path | None = None,
        timeout: float = 60.0 * 6,
    ) -> ToolDataOutput:
        if not tcl_script_fp.exists():
            raise FileNotFoundError(f"TCL script not found: {tcl_script_fp}")
        if work_dir is None:
            work_dir = tcl_script_fp.parent

        t_0 = time.monotonic()
        p = subprocess.Popen(
            [self.vitis_hls_bin, "-f", tcl_script_fp.resolve()],
            cwd=work_dir.resolve(),
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )
        try:
            stdout, stderr = p.communicate(timeout=timeout)
            return_code = p.returncode
            timeout_reached = False
        except subprocess.TimeoutExpired:
            self._terminate_process_tree(p.pid)
            stdout, stderr = p.communicate()
            return_code = -1
            timeout_reached = True

        t_1 = time.monotonic()
        dt = t_1 - t_0

        return ToolDataOutput(
            data_execution=ExecutionData(
                return_code=return_code,
                stdout=stdout,
                stderr=stderr,
                t0=t_0,
                t1=t_1,
                execution_time=dt,
                timeout=timeout_reached,
            ),
            data_tool=None,
        )
