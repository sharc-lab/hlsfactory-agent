from __future__ import annotations

import os
import re
import signal
import subprocess
from pathlib import Path

from hlsfactory_agent.spec import SynthResult, TargetSpec


def _expand(arg: str, env: dict[str, str]) -> str:
    return re.sub(r"\$(\w+)", lambda m: env.get(m.group(1), m.group(0)), arg)


def run_synth(target: TargetSpec, dir_run: Path) -> SynthResult:
    """Run the target's synthesis tool in dir_run and parse the result. Target values override the environment."""
    if not target.synth_command or target.parse_report is None:
        raise NotImplementedError(f"target `{target.name}` has no synthesis step")
    env = {**os.environ, **target.synth_env}
    for name, text in target.synth_files.items():
        (dir_run / name).write_text(text, encoding="utf-8")
    cmd = [_expand(a, env) for a in target.synth_command]
    with open(dir_run / "synth_stdout.log", "w", encoding="utf-8") as out:
        proc = subprocess.Popen(cmd, cwd=dir_run, env=env, stdout=out, stderr=subprocess.STDOUT, start_new_session=True)
        try:
            proc.wait(timeout=target.synth_timeout_s)
        except subprocess.TimeoutExpired:
            os.killpg(proc.pid, signal.SIGKILL)
            proc.wait()
            return SynthResult("TIMEOUT", failure_class="timeout")
    result = target.parse_report(dir_run)
    if result.status == "PASS" and target.synth_success_marker and not any(dir_run.glob(target.synth_success_marker)):
        return SynthResult("FAILED", first_error=f"missing {target.synth_success_marker}", failure_class="no-output")
    return result
