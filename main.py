import argparse
import subprocess
import sys
import json
import uuid
import os
from datetime import datetime, timezone
from pathlib import Path

from minisweagent.agents.default import DefaultAgent
from minisweagent.models.openrouter_model import OpenRouterModel
from minisweagent.environments.docker import DockerEnvironment
from minisweagent.config import get_config_from_spec


def load_env(env_path: Path) -> None:
    if not env_path.is_file():
        return
    with open(env_path) as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#") or "=" not in line:
                continue
            key, _, value = line.partition("=")
            os.environ.setdefault(key.strip(), value.strip().strip("'\""))


def build_prompt(repo_url: str) -> str:
    return (
        f"Process the HLS repository at '{repo_url}' and extract all HLS designs to '/output'.\n"
        "\n"
        "Execute the complete HLSFactory pipeline:\n"
        f"1. Clone the repository from '{repo_url}' into /workspace/repo\n"
        "2. Analyze the repository and identify all HLS designs\n"
        "3. Extract each design into its own folder under /output\n"
        "4. Find or generate testbenches for each design\n"
        "5. Generate documentation for each design\n"
        "6. Compile each design's source code using clang to verify it compiles without errors. "
        "Record any errors in a compile_log.txt inside the design folder.\n"
        "7. Generate TCL synthesis scripts\n"
        "8. Create the final manifest\n"
        "9. Put all the design folders within a parent folder with the name of repository under /output\n"
        "\n"
        "The HLS stub headers are available at: /workspace/stubs\n"
        "\n"
        "Work through each stage systematically and process ALL designs found. "
        "Do not skip any design, testbench creation, or compilation step — these are all required.\n"
        "\n"
        "IMPORTANT: Start executing immediately. Do not just list steps — run the actual commands. "
        "Begin by cloning the repository now."
    )

def main():
    # python script arguments
    parser = argparse.ArgumentParser(
        description="HLSFactory mini-swe-agent — run the HLS pipeline via Docker."
    )
    parser.add_argument(
        "source_repo",
        help="URL or local path to the HLS repository to process",
    )
    parser.add_argument(
        "-o", "--output-dir",
        default="./output",
        help="Host directory for results (default: ./output)",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Print commands without executing them",
    )
    args = parser.parse_args()

    load_env(Path(__file__).resolve().parent / ".env")
    output_dir = Path(args.output_dir)
    repo_url = args.source_repo
    repo_name = repo_url.rstrip("/").split("/")[-1].removesuffix(".git")


    # 1. build docker image
    print("Building Docker image...")
    subprocess.run(["docker", "build", "-t", "hlsfactory", "."], cwd=Path(__file__).parent, check=True)

    # 2. instantiate DockerEnvironment for mini swe agent
    env = DockerEnvironment(
        image = "hlsfactory",
        cwd="/workspace",
        forward_env=["OPENROUTER_API_KEY"],
    )

    # 3. instantiate the agent
    agent_config = get_config_from_spec("default")["agent"]
    agent = DefaultAgent(
        OpenRouterModel(model_name="moonshotai/kimi-k2.5"),
        env,
        **agent_config,
    )

    # 4. run the agent
    result = agent.run(build_prompt(repo_url))

    # 5. save agent results in trajectory file
    run_id = str(uuid.uuid4())
    output_dir.mkdir(parents=True, exist_ok=True)

    # creates the mini swe agent raw json file - we must provide a path
    traj_path = output_dir / f"{repo_name}_{run_id}.traj.json"
    agent.save(traj_path)

    # 6. our benchmark json
    benchmark = {
          "run_id": run_id,
          "timestamp": datetime.now(timezone.utc).isoformat(),
          "repo_url": repo_url,
          "model": "moonshotai/kimi-k2.5",
          "exit_status": result.get("exit_status"),
          "total_cost_usd": agent.cost,
          "total_api_calls": agent.n_calls,
          "traj_file": traj_path.name,
    }

    # will probably add more code to evaluate what tools it called, etc.

    benchmark_path = output_dir / f"{repo_name}_{run_id}_benchmark.json"
    benchmark_path.write_text(json.dumps(benchmark, indent=2))

    print(f"Trajectory: {traj_path}")
    print(f"Benchmark:  {benchmark_path}")

    # 7. copy /output from the container to the host
    container_id = env.container_id

    if not container_id:
        print("Warning: could not find container ID, skipping output copy.", file=sys.stderr)
    else:
        subprocess.run(["docker", "cp", f"{container_id}:/output/.", str(output_dir)], check=True)
        env.cleanup()
        print(f"HLS results copied to {output_dir}")


if __name__ == "__main__":
    main()
