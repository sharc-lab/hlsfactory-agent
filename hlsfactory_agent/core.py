import json
import os
from pathlib import Path
import shlex
import shutil
import subprocess

import docker
from docker.models.containers import Container

from github_fast_downloader import GithubFastDownloader

from hlsfactory_agent.prompt import build_prompt
from hlsfactory_agent.utils import Timer, load_jsonl_text

DIR_CURRENT = Path(__file__).resolve().parent
DIR_VITIS_HLS_INCLUDE = DIR_CURRENT / "vitis_hls_include"
assert DIR_VITIS_HLS_INCLUDE.exists()

DOCKER_IMAGE_NAME = "hlsfactory-agent"

class HLSFactoryAgentRun:
    def __init__(
        self,
        run_id: str,
        repo_id: str,
        dir_work: Path,
        model_name: str,
        api_key: str,
        prompt_task: str | None = None,
        prompt_system: str | None = None,
        docker_image_name: str | None = DOCKER_IMAGE_NAME,
    ):
        self.run_id = run_id
        self.repo_id = repo_id
        self.dir_work = dir_work

        self.model_name = model_name
        self.api_key = api_key

        self.prompt_task = prompt_task
        self.prompt_system = prompt_system

        self.docker_image_name = docker_image_name

    def run(self) -> None:
        print(f"Running HLSFactoryAgent for repo `{self.repo_id}`")

        run_data: dict[str, int | str | float | None | dict | list] = {}
        run_data["run_id"] = self.run_id
        run_data["repo_id"] = self.repo_id
        run_data["model_name"] = self.model_name
        run_data["prompt_task"] = self.prompt_task
        run_data["prompt_system"] = self.prompt_system
        run_data["docker_image_name"] = self.docker_image_name

        if self.dir_work.exists():
            shutil.rmtree(self.dir_work)
        self.dir_work.mkdir(parents=True, exist_ok=True)


        dir_run_area = self.dir_work / "run_area"
        dir_run_area.mkdir(parents=True, exist_ok=True)

        dir_pi_config_in_work = dir_run_area / ".pi"
        dir_pi_config_in_work.mkdir(parents=True, exist_ok=True)

        pi_settings = {
                "defaultProvider": "openrouter",
                "defaultModel": self.model_name,
                "sessionDir": ".pi/sessions",
            }
        (dir_pi_config_in_work / "settings.json").write_text(
            json.dumps(pi_settings, indent=4)
        )
        dir_sessions = dir_pi_config_in_work / "sessions"
        dir_sessions.mkdir(parents=True, exist_ok=True)
        os.chmod(dir_pi_config_in_work, 0o777)
        os.chmod(dir_sessions, 0o777)

        # git clone into dir_run_area
        repo_id_split = self.repo_id.split("/")
        if len(repo_id_split) == 2:
            repo_owner = repo_id_split[0]
            repo_name = repo_id_split[1]
        else:
            raise ValueError(f"Invalid repo ID: {self.repo_id}")

        p_clone = subprocess.run(["git", "clone", f"https://github.com/{repo_owner}/{repo_name}.git"], cwd=dir_run_area, capture_output=True, text=True)
        if p_clone.returncode != 0:
            raise RuntimeError(f"Failed to clone repo:\nstderr: {p_clone.stderr}\nstdout: {p_clone.stdout}")

        repo_dir = dir_run_area / repo_name
        assert repo_dir.exists()

        shutil.copytree(DIR_VITIS_HLS_INCLUDE, dir_run_area / "vitis_hls_include")

        dir_output_hls_designs = dir_run_area / "output_hls_designs"
        dir_output_hls_designs.mkdir(parents=True, exist_ok=True)

        client = docker.from_env()
        container: Container = client.containers.run(
            image=self.docker_image_name,
            command="sleep 1h",
            detach=True,
            volumes={
                str(dir_run_area.resolve()): {
                    "bind": "/workspace/run_area",
                    "mode": "rw",
                },
            },
        )

        prompt = build_prompt(repo_name)
        prompt_escaped = shlex.quote(prompt)
        cmd = f"umask 000 && pi -p {prompt_escaped}"
        exit_code, output_agent = container.exec_run(
            ["sh", "-lc", cmd],
            environment={"OPENROUTER_API_KEY": self.api_key},
            workdir="/workspace/run_area",
        )
        if exit_code != 0:
            raise RuntimeError(f"Failed to get pi agent output: {output_agent}")

        dir_sessions = dir_pi_config_in_work / "sessions"
        if not dir_sessions.exists():
            raise RuntimeError(
                f"No sessions directory found in {dir_pi_config_in_work}"
            )
        # find session file
        session_file = next(dir_sessions.glob("*.jsonl"), None)
        if session_file is None:
            raise RuntimeError(f"No session file found in {dir_sessions}")
        session_data = load_jsonl_text(session_file.read_text())
        run_data["session_data"] = session_data

        exit_code, _ = container.exec_run(
            [
                "sh",
                "-lc",
                f"umask 000 && pi --export /workspace/run_area/.pi/sessions/{session_file.name}  /workspace/run_area/.pi/sessions/{session_file.name.replace('.jsonl', '.html')}",
            ],
            workdir="/workspace/run_area",
        )
        if exit_code != 0:
            raise RuntimeError("Failed to convert session data to html")

        container.stop()
        container.remove(force=True)

        (self.dir_work / "run_data.json").write_text(json.dumps(run_data, indent=4))

        

