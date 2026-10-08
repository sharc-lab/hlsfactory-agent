from __future__ import annotations

import hashlib
import json
import os
import re
import shlex
import shutil
from dataclasses import asdict
from pathlib import Path

from hlsfactory_agent.spec import SynthResult, TargetSpec
from hlsfactory_agent.tools import installation

DOCKER_IMAGE_NAME = "hlsfactory-agent"
CONTAINER_WORK = "/work"


def _expand(arg: str, env: dict[str, str]) -> str:
    return re.sub(r"\$(\w+)", lambda m: env.get(m.group(1), m.group(0)), arg)


def run_synth(target: TargetSpec, dir_run: Path, image: str = DOCKER_IMAGE_NAME) -> SynthResult:
    """Run the target's tool on dir_run inside a fresh container: the tool install mounted read-only, dir_run
    mounted at /work, the process owned by the calling user. The agent never shares this container."""
    if not target.synth_command or target.parse_report is None:
        raise NotImplementedError(f"target `{target.name}` has no synthesis step")
    inst = installation(target.tool)
    if inst is None:
        return SynthResult("FAILED", first_error=f"no {target.tool} installation found on this host", failure_class="tool-missing")
    for name, text in target.synth_files.items():
        (dir_run / name).write_text(text, encoding="utf-8")
    env = {**inst.environment, **target.synth_env, "HOME": CONTAINER_WORK}
    cmd = " ".join(shlex.quote(_expand(a, env)) for a in target.synth_command)
    import docker  # local import: the unit tests never touch a daemon

    client = docker.from_env()
    container = client.containers.run(
        image=image,
        command=["bash", "-c", cmd],
        volumes={**inst.volumes, str(Path(dir_run).resolve()): {"bind": CONTAINER_WORK, "mode": "rw"}},
        environment=env,
        user=f"{os.getuid()}:{os.getgid()}",
        working_dir=CONTAINER_WORK,
        detach=True,
    )
    try:
        try:
            container.wait(timeout=target.synth_timeout_s)
        except Exception:  # requests' read timeout: the budget is spent
            container.kill()
            return SynthResult("TIMEOUT", failure_class="timeout")
        (dir_run / "synth_stdout.log").write_bytes(container.logs())
    finally:
        try:
            container.remove(force=True)
        except Exception:
            pass
    result = target.parse_report(dir_run)
    if result.status == "PASS" and target.synth_success_marker and not any(dir_run.glob(target.synth_success_marker)):
        return SynthResult("FAILED", first_error=f"missing {target.synth_success_marker}", failure_class="no-output")
    return result


def run_synth_degraded(target: TargetSpec, dir_design: Path, dir_work: Path, dir_cache: Path, rounds: int = 2) -> list[dict]:
    """After a failed run_synth_cached, apply the target's degrade step to the failed run and synthesize again,
    up to `rounds` times or until no step applies. Returns one record per round; the caller keeps them apart
    from the original result."""
    out: list[dict] = []
    dir_failed = dir_work
    for n in range(1, rounds + 1):
        if target.degrade_step is None:
            break
        dir_next = dir_work.parent / f"{dir_work.name}_degrade{n}"
        if dir_next.exists():
            shutil.rmtree(dir_next)
        step = target.degrade_step(dir_failed, dir_next)
        if step is None:
            break
        result = run_synth_cached(target, dir_next, dir_next / "run", dir_cache)
        out.append({"step": step, **asdict(result)})
        if result.status == "PASS":
            break
        dir_failed = dir_next / "run"
    return out


def design_hash(dir_design: Path) -> str:
    """Content hash of every regular file under the design, so an unchanged design is never re-synthesized."""
    h = hashlib.sha256()
    for p in sorted(q for q in Path(dir_design).rglob("*") if q.is_file()):
        h.update(str(p.relative_to(dir_design)).encode("utf-8"))
        h.update(p.read_bytes())
    return h.hexdigest()[:16]


def run_synth_cached(target: TargetSpec, dir_design: Path, dir_work: Path, dir_cache: Path) -> SynthResult:
    """run_synth on a fresh copy of dir_design at dir_work, memoized in dir_cache by target and content hash."""
    key = dir_cache / f"{target.name}-{design_hash(dir_design)}-{target.synth_timeout_s}s.json"
    if key.exists():
        return SynthResult(**json.loads(key.read_text(encoding="utf-8")))
    if dir_work.exists():
        shutil.rmtree(dir_work)
    shutil.copytree(dir_design, dir_work)
    result = run_synth(target, dir_work)
    dir_cache.mkdir(parents=True, exist_ok=True)
    key.write_text(json.dumps(asdict(result), indent=1), encoding="utf-8")
    return result
