"""Docker smoke test for the paper-filter agent: checks every moving part except a real paper.

    PYTHONPATH=. .venv/bin/python exp/paper_filter/smoke_test.py [--image NAME] [--skip-llm]

Runs the same container setup as run_agent.py (host uid, HOME in the bind mount,
host.docker.internal -> host-gateway, proxy on the Docker bridge address) and checks, in order:
  1. keys in .env              2. Docker daemon + image     3. tools inside the image
  4. container -> host proxy (GitHub with token, auth rejected without the secret)
  5. web search (Exa) through the proxy                     6. files written in the
     container are owned by you on the host                 7. pi -> OpenRouter -> model
Stops at the first failure and prints what to look at. Costs about $0.01 (one Exa query,
one tiny model call). Leaves nothing behind except an `smoke/` folder under the runs dir.
"""

import argparse
import json
import os
import secrets
import shutil
import sys
from pathlib import Path

from dotenv import dotenv_values

from hlsfactory_agent.paper_filter.github import load_token
from hlsfactory_agent.paper_filter.proxy import ExaProvider, ProxyConfig, ProxyServer, bridge_address
from hlsfactory_agent.paper_filter.registry import Registry
from hlsfactory_agent.paper_filter.runner import RunConfig, start_container, write_pi_settings
from hlsfactory_agent.paper_filter.workfolder import DIR_TOOLS_SRC

DIR_CURRENT = Path(__file__).resolve().parent
DIR_ROOT = DIR_CURRENT.parent.parent
MODEL = "deepseek/deepseek-v4-flash"

ap = argparse.ArgumentParser()
ap.add_argument("--image", default="paper-filter-agent")
ap.add_argument("--model", default=MODEL)
ap.add_argument("--run-dir", type=Path, default=DIR_CURRENT / "runs" / "smoke")
ap.add_argument("--skip-llm", action="store_true", help="skip the pi/OpenRouter step")
args = ap.parse_args()


def ok(step: str, detail: str = "") -> None:
    print(f"[ OK ] {step}" + (f": {detail}" if detail else ""))


def fail(step: str, detail: str, hint: str) -> None:
    print(f"[FAIL] {step}: {detail}\n       hint: {hint}")
    sys.exit(1)


# 1. keys -------------------------------------------------------------------------------
env = dotenv_values(DIR_ROOT / ".env")
token = load_token(DIR_ROOT / ".env")
missing = [k for k, v in (("OPENROUTER_API_KEY", env.get("OPENROUTER_API_KEY")),
                          ("GITHUB_TOKEN", token), ("EXA_KEY", env.get("EXA_KEY"))) if not v]  # fmt: skip
if missing:
    fail("keys", f"missing from {DIR_ROOT / '.env'}: {', '.join(missing)}",
         "copy .env from your laptop (it is gitignored)")  # fmt: skip
ok("keys", "OPENROUTER_API_KEY, GITHUB_TOKEN, EXA_KEY present")


# 2. docker + image ---------------------------------------------------------------------
try:
    import docker

    client = docker.from_env()
    client.ping()
except Exception as e:
    fail("docker daemon", str(e), "is Docker running, and are you in the `docker` group? "
         "(`docker run --rm hello-world` must work without sudo)")  # fmt: skip
try:
    client.images.get(args.image)
except docker.errors.ImageNotFound:
    fail("image", f"{args.image!r} not found", "run docker_images/paper_filter/build.sh")
ok("docker", f"daemon reachable, image {args.image!r} present")


# shared setup: run area with the agent's tools, proxy, container ----------------------
run_area = args.run_dir / "run_area"
if run_area.exists():
    shutil.rmtree(run_area)
(run_area / "tools").mkdir(parents=True)
for src in DIR_TOOLS_SRC.glob("*.sh"):
    shutil.copy(src, run_area / "tools" / src.name)
    (run_area / "tools" / src.name).chmod(0o755)

secret = secrets.token_hex(16)
host = bridge_address()
proxy = ProxyServer(
    ProxyConfig(secret=secret, token=token, log_path=args.run_dir / "proxy_calls.jsonl"),
    Registry(args.run_dir / "registry.sqlite"),
    host=host,
    web_provider=ExaProvider(env["EXA_KEY"]),
).start()
cfg = RunConfig(model=args.model, api_key=env["OPENROUTER_API_KEY"], image=args.image,
                proxy_url=f"http://host.docker.internal:{proxy.port}", proxy_secret=secret)  # fmt: skip
print(f"       proxy on {host}:{proxy.port}; container sees {cfg.proxy_url}")
write_pi_settings(run_area, cfg.model)
container = start_container(cfg, run_area, "smoke-test")


def sh(cmd: str, env_extra: dict | None = None) -> tuple[int, str]:
    code, out = container.exec_run(["sh", "-lc", cmd], environment=env_extra or {})
    return code, (out or b"").decode(errors="replace").strip()


try:
    # 3. tools inside the image ---------------------------------------------------------
    code, out = sh("id -u; echo $HOME; pi --version; pdftotext -v 2>&1 | head -1; "
                   "python3 -c 'import pymupdf; print(\"pymupdf\", pymupdf.__version__)'; "
                   "git --version; jq --version; rg --version | head -1")  # fmt: skip
    if code != 0:
        fail("image tools", out, "rebuild the image; see which command above is missing")
    ok("image tools", " | ".join(out.splitlines()))

    # 4. container -> host proxy -> GitHub ----------------------------------------------
    code, out = sh("tools/gh_api.sh rate_limit")
    try:
        res = json.loads(out)["resources"]
    except (ValueError, KeyError):
        fail("proxy (GitHub)", out[:300] or f"exit {code}",
             f"container cannot reach {host}:{proxy.port}. Linux: check a firewall (ufw/"
             "firewalld) allows docker0 -> host, and that Docker >= 20.10 (host-gateway). "
             "Mac: Docker Desktop must be running.")  # fmt: skip
    cs = res["code_search"]
    if cs["limit"] < 10:
        fail("proxy (GitHub)", f"code_search limit {cs['limit']}", "token not applied; check GITHUB_TOKEN")
    ok("proxy (GitHub)", f"core {res['core']['remaining']}/{res['core']['limit']}, "
       f"code_search {cs['limit']}/min (token applied)")  # fmt: skip
    code, out = sh('curl -s -o /dev/null -w "%{http_code}" "$PROXY_URL/github/rate_limit"')
    if out != "403":
        fail("proxy auth", f"request without secret got {out}", "proxy secret check broken")
    ok("proxy auth", "request without the secret is refused (403)")

    # 5. web search ---------------------------------------------------------------------
    code, out = sh("tools/web_search.sh 'InTAR inter-task auto-reconfigurable accelerator github'")
    try:
        urls = [r["url"] for r in json.loads(out)["results"]]
    except (ValueError, KeyError):
        fail("web search (Exa)", out[:300], "check EXA_KEY and the account's credits")
    ok("web search (Exa)", f"{len(urls)} results, first: {urls[0] if urls else '-'}")

    # 6. file ownership in the bind mount -----------------------------------------------
    sh("umask 000 && echo hi > owner_check.txt")
    f = run_area / "owner_check.txt"

    if not f.exists():
        fail("bind mount", "file written in the container is not visible on the host",
             "the run dir must be on a local disk Docker can mount (not every NFS path works)")  # fmt: skip
    if hasattr(os, "getuid") and f.stat().st_uid != os.getuid():
        fail("bind mount", f"file owned by uid {f.stat().st_uid}, you are {os.getuid()}",
             "container user mapping differs (rootless Docker or userns-remap); tell Claude")  # fmt: skip
    ok("bind mount", "container writes as your uid")

    # 7. pi -> OpenRouter ---------------------------------------------------------------
    if args.skip_llm:
        print("[SKIP] pi / OpenRouter")
    else:
        code, out = sh("timeout 180 pi -p 'Reply with exactly the word: pong'",
                       {"OPENROUTER_API_KEY": cfg.api_key})  # fmt: skip
        if code != 0 or "pong" not in out.lower():
            fail("pi / OpenRouter", f"exit {code}: {out[-500:]}",
                 "124 = timeout (OpenRouter unreachable from the container?); 401 = bad key; "
                 f"unknown model = check {args.model!r} on openrouter.ai")  # fmt: skip
        sessions = list((run_area / ".pi" / "sessions").glob("*.jsonl"))
        ok("pi / OpenRouter", f"model replied {out!r}; session file written: {bool(sessions)}")
finally:
    container.remove(force=True)
    proxy.shutdown()


print("\nAll checks passed. Next: run one real paper (see SERVER_TEST.md, step 6).")
