"""Run statistics: per-stage wall times, a host resource sampler and retry accounting. `run_repo.py` writes
them next to the runs (`<runs>/stats/<repo>.json`, `.resources.jsonl`); `exp/run_translate/stats_report.py`
turns them into docs/RUN_STATISTICS.md."""
from __future__ import annotations

import json
import os
import threading
import time
from collections import Counter
from contextlib import contextmanager
from pathlib import Path

DOCKER_IMAGE_NAME = "hlsfactory-agent"


class StageTimer:
    def __init__(self) -> None:
        self.seconds: dict[str, float] = {}

    @contextmanager
    def __call__(self, name: str):
        t0 = time.monotonic()
        try:
            yield
        finally:
            self.seconds[name] = round(self.seconds.get(name, 0.0) + time.monotonic() - t0, 1)


def _meminfo() -> dict[str, int]:
    out = {}
    for line in Path("/proc/meminfo").read_text(encoding="utf-8").splitlines():
        k, v = line.split(":", 1)
        if k in ("MemTotal", "MemAvailable"):
            out[k] = int(v.split()[0])
    return out


def host_info() -> dict:
    return {"hostname": os.uname().nodename, "cpus": os.cpu_count(), "mem_total_gb": round(_meminfo()["MemTotal"] / 2**20, 1)}


def sample(image: str = DOCKER_IMAGE_NAME) -> dict:
    """One host-wide sample: 1-minute load, memory in use, running containers of the pipeline's image."""
    mem = _meminfo()
    try:
        import docker

        containers = len(docker.from_env().containers.list(filters={"ancestor": image}))
    except Exception:
        containers = None
    return {
        "ts": round(time.time()),
        "load1": float(Path("/proc/loadavg").read_text(encoding="utf-8").split()[0]),
        "mem_used_gb": round((mem["MemTotal"] - mem["MemAvailable"]) / 2**20, 1),
        "containers": containers,
    }


class ResourceSampler(threading.Thread):
    """Appends a sample every `interval_s` to `path` until stopped."""

    def __init__(self, path: Path, interval_s: float = 30.0) -> None:
        super().__init__(daemon=True)
        self.path, self.interval_s, self._stop_event = Path(path), interval_s, threading.Event()
        self.samples: list[dict] = []

    def run(self) -> None:
        self.path.parent.mkdir(parents=True, exist_ok=True)
        with self.path.open("a", encoding="utf-8") as f:
            while not self._stop_event.is_set():
                s = sample()
                self.samples.append(s)
                f.write(json.dumps(s) + "\n")
                f.flush()
                self._stop_event.wait(self.interval_s)

    def stop(self) -> None:
        self._stop_event.set()
        self.join(timeout=5)

    def summary(self) -> dict:
        return summarize_samples(self.samples)


def summarize_samples(samples: list[dict]) -> dict:
    out: dict = {"samples": len(samples)}
    for k in ("load1", "mem_used_gb", "containers"):
        vals = [s[k] for s in samples if s.get(k) is not None]
        out[f"{k}_mean"] = round(sum(vals) / len(vals), 1) if vals else None
        out[f"{k}_max"] = max(vals) if vals else None
    return out


def usage_of_events(events: list[dict]) -> dict:
    """Tokens, cost, tool calls and wall time of one Pi session (its event list)."""
    out = {"wall_s": None, "tokens_in": 0, "tokens_out": 0, "cost_usd": 0.0, "tool_calls": 0}
    t0 = t1 = None
    for e in events:
        ts = e.get("timestamp")
        if ts:
            t0 = t0 or ts
            t1 = ts
        m = e.get("message") or {}
        u = m.get("usage") or {}
        if u:
            out["tokens_in"] += int(u.get("input", u.get("input_tokens", 0)) or 0)
            out["tokens_out"] += int(u.get("output", u.get("output_tokens", 0)) or 0)
            c = u.get("cost") or 0
            out["cost_usd"] += float(c.get("total", 0) if isinstance(c, dict) else c)
        content = m.get("content")
        if isinstance(content, list):
            out["tool_calls"] += sum(1 for part in content if isinstance(part, dict) and part.get("type") == "toolCall")
    if t0 and t1:
        from datetime import datetime

        try:
            out["wall_s"] = round((datetime.strptime(t1, "%Y-%m-%dT%H:%M:%S.%fZ") - datetime.strptime(t0, "%Y-%m-%dT%H:%M:%S.%fZ")).total_seconds())
        except ValueError:
            pass
    out["cost_usd"] = round(out["cost_usd"], 4)
    return out


def agent_usage(dir_run: Path) -> dict:
    """Usage summed over every agent attempt of a run (the session files `run_data.json` lists), with the
    per-attempt figures under `attempts`; falls back to the last attempt's events for older runs."""
    f = Path(dir_run) / "run_data.json"
    total = {"wall_s": None, "tokens_in": 0, "tokens_out": 0, "cost_usd": 0.0, "tool_calls": 0, "attempts": []}
    if not f.exists():
        return total
    try:
        data = json.loads(f.read_text(encoding="utf-8"))
    except Exception:
        return total
    per: list[dict] = []
    for s in data.get("sessions") or []:
        sf = Path(dir_run) / "run_area" / ".pi" / "sessions" / s["file"] if s.get("file") else None
        if sf and sf.exists():
            events = []
            for line in sf.read_text(encoding="utf-8", errors="replace").splitlines():
                try:
                    events.append(json.loads(line))
                except ValueError:
                    continue
            per.append({"attempt": s.get("attempt"), **usage_of_events(events)})
    if not per:
        per = [{"attempt": 1, **usage_of_events(data.get("session_data") or [])}]
    for u in per:
        for k in ("tokens_in", "tokens_out", "cost_usd", "tool_calls"):
            total[k] += u[k]
    walls = [u["wall_s"] for u in per if u["wall_s"] is not None]
    total["wall_s"] = sum(walls) if walls else None
    total["cost_usd"] = round(total["cost_usd"], 4)
    total["attempts"] = per
    return total


def failure_category(failures: list[str]) -> str:
    """One label per attempt from the harness's own messages; an empty list with no pass is the container
    (compile or testbench) side."""
    if not failures:
        return "testbench"
    f = failures[0]
    for key, name in (("outside the residue", "fence"), ("source tool's dialect", "dialect"), ("top marker", "top-marker"),
                      ("directive", "directive"), ("missing", "missing-file"), ("driver", "driver"), ("harness error", "harness")):
        if key in f:
            return name
    return "other"


def retry_stats(rows: list[dict]) -> dict:
    """From each row's `attempts` list (one entry per agent attempt with `passed` and `failures`): at which
    attempt designs first pass, how each failure category fares on the next attempt, and the designs that fail
    every attempt with the same failures (thrashing)."""
    first_pass: Counter = Counter()
    transitions: Counter = Counter()
    thrashing, never = [], []
    for r in rows:
        att = r.get("attempts") or []
        if not att:
            continue
        k = next((a["attempt"] for a in att if a.get("passed")), None)
        if k:
            first_pass[k] += 1
        else:
            never.append(r["design"])
            sigs = {tuple(a.get("failures") or []) for a in att}
            if len(att) >= 2 and len(sigs) == 1:
                thrashing.append({"design": r["design"], "attempts": len(att), "category": failure_category(att[0].get("failures") or []), "failure": (att[0].get("failures") or [""])[0][:120]})
        for a, b in zip(att, att[1:]):
            cat = failure_category(a.get("failures") or [])
            nxt = "pass" if b.get("passed") else failure_category(b.get("failures") or [])
            transitions[(cat, "pass" if nxt == "pass" else ("same" if nxt == cat else nxt))] += 1
    return {"first_pass": dict(sorted(first_pass.items())), "never": never, "thrashing": thrashing,
            "transitions": {f"{a}->{b}": n for (a, b), n in sorted(transitions.items())}, "designs": sum(1 for r in rows if r.get("attempts"))}
