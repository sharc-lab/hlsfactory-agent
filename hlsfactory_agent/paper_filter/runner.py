"""Run the repo-discovery agent for one paper (modelled on hlsfactory_agent/core.py).

Top-level function so joblib can ship it to worker processes. Returns the final per-paper
record; the orchestrator (single writer) merges it into the registry.
"""

import json
import os
import shlex
import traceback
from dataclasses import dataclass
from pathlib import Path

from hlsfactory_agent.paper_filter.extract import run_prepass
from hlsfactory_agent.paper_filter.manifest import PaperMeta
from hlsfactory_agent.paper_filter.postprocess import finalize
from hlsfactory_agent.paper_filter.prompt import build_agent_prompt
from hlsfactory_agent.paper_filter.workfolder import prepare_work_folder

CONTAINER_RUN_AREA = "/workspace/run_area"


@dataclass
class RunConfig:
    model: str
    api_key: str | None = None  # OpenRouter; goes into the container (pi needs it)
    image: str = "paper-filter-agent"
    proxy_url: str = ""  # as seen from inside the container
    proxy_secret: str = ""
    github_token: str | None = None  # host pre-pass only; never passed to containers
    timeout_s: int = 1800
    mem_limit: str = "4g"
    cpus: float = 1.0
    dry_run: bool = False


def _failure(meta: PaperMeta, status: str, error: str) -> dict:
    return {
        "paper_id": meta.paper_id,
        "title": meta.title,
        "status": status,
        "outcome": None,
        "repos": [],
        "error": error,
    }


def write_pi_settings(run_area: Path, model: str) -> None:
    """pi config inside the bind mount; HOME is also there because the uid has no home."""
    pi_dir = run_area / ".pi"
    (pi_dir / "sessions").mkdir(parents=True, exist_ok=True)
    (pi_dir / "settings.json").write_text(
        json.dumps(
            {"defaultProvider": "openrouter", "defaultModel": model, "sessionDir": ".pi/sessions"},
            indent=4,
        )
    )
    (run_area / "home").mkdir(exist_ok=True)


def start_container(cfg: RunConfig, run_area: Path, paper_id: str):
    """Start the agent container idle (`sleep infinity`); the caller execs into it and removes it.

    Runs as the host user so files in the bind mount are not root-owned. A writable HOME is
    required, hence HOME inside the run area.
    """
    import docker  # lazy: only needed for real runs

    user = f"{os.getuid()}:{os.getgid()}" if hasattr(os, "getuid") else None
    return docker.from_env().containers.run(
        image=cfg.image,
        command="sleep infinity",
        detach=True,
        user=user,
        working_dir=CONTAINER_RUN_AREA,
        volumes={str(run_area.resolve()): {"bind": CONTAINER_RUN_AREA, "mode": "rw"}},
        environment={
            "HOME": f"{CONTAINER_RUN_AREA}/home",
            "PROXY_URL": cfg.proxy_url,
            "PROXY_SECRET": cfg.proxy_secret,
            "GH_API_BASE": f"{cfg.proxy_url}/github",
            "PAPER_ID": paper_id,
        },
        extra_hosts={"host.docker.internal": "host-gateway"},
        # run_area = <run_dir>/work/<paper_id>/run_area; labels let `run_agent.py --status` find these
        labels={"paper_filter.run_dir": str(run_area.resolve().parents[2]), "paper_filter.paper": paper_id},
        mem_limit=cfg.mem_limit,
        nano_cpus=int(cfg.cpus * 1e9),
    )


def _run_agent(cfg: RunConfig, meta: PaperMeta, run_area: Path, prompt: str) -> tuple[str, str]:
    """Run pi in a container. Returns (status, detail); output lands in run_area/output."""
    write_pi_settings(run_area, cfg.model)
    pi_dir = run_area / ".pi"
    container = start_container(cfg, run_area, meta.paper_id)
    try:
        cmd = f"umask 000 && timeout {cfg.timeout_s} pi -p {shlex.quote(prompt)}"
        exit_code, out = container.exec_run(
            ["sh", "-lc", cmd],
            environment={"OPENROUTER_API_KEY": cfg.api_key or ""},
            workdir=CONTAINER_RUN_AREA,
        )
        (run_area.parent / "agent_output.txt").write_bytes(out or b"")
        session = next((pi_dir / "sessions").glob("*.jsonl"), None)
        if session is not None:  # transcript export is best-effort
            container.exec_run(
                ["sh", "-lc",
                 f"pi --export {CONTAINER_RUN_AREA}/.pi/sessions/{session.name} "
                 f"{CONTAINER_RUN_AREA}/.pi/sessions/{session.stem}.html"],
                workdir=CONTAINER_RUN_AREA,
            )  # fmt: skip
        if exit_code == 124:
            return "timeout", f"agent exceeded {cfg.timeout_s}s"
        if exit_code != 0:
            return "agent_failed", f"pi exited {exit_code}"
        return "ok", ""
    finally:
        container.remove(force=True)


def run_one(meta: PaperMeta, cfg: RunConfig, dir_runs: Path, snapshot: dict) -> dict:
    dir_paper = dir_runs / "work" / meta.paper_id
    dir_papers_out = dir_runs / "papers"
    dir_papers_out.mkdir(parents=True, exist_ok=True)
    try:
        prepass, doc = run_prepass(
            meta.pdf, meta.title, cfg.github_token, dir_runs / "_github_cache"
        )
        run_area = prepare_work_folder(dir_paper / "run_area", meta, prepass, doc, snapshot)
    except Exception as e:  # one bad PDF must not stop the batch
        final = _failure(meta, "prepass_failed", f"{e}\n{traceback.format_exc()}")
        (dir_papers_out / f"{meta.paper_id}.json").write_text(json.dumps(final, indent=2))
        return final

    prompt = build_agent_prompt(
        meta.paper_id, meta.title, meta.authors, meta.year, meta.venue, meta.doi
    )
    (dir_paper / "prompt.txt").write_text(prompt)

    if cfg.dry_run:
        final = _failure(meta, "prepared", "dry run: container not started")
    else:
        try:
            status, detail = _run_agent(cfg, meta, run_area, prompt)
        except Exception as e:
            status, detail = "agent_failed", f"{e}\n{traceback.format_exc()}"
        result_file = run_area / "output" / "result.json"
        raw = None
        if result_file.exists():
            try:
                raw = json.loads(result_file.read_text())
            except json.JSONDecodeError as e:
                detail = f"{detail}; result.json invalid: {e}"
        if raw is None:
            final = _failure(meta, "no_result" if status == "ok" else status, detail)
        else:
            final = finalize(raw, meta.paper_id, meta.title, doc.pages, prepass, snapshot)
            if status != "ok":
                final["problems"].append(f"agent {status}: {detail}; used result.json anyway")
    final["meta"] = {k: v for k, v in meta.to_json().items() if k != "pdf"}
    (dir_papers_out / f"{meta.paper_id}.json").write_text(json.dumps(final, indent=2))
    return final
