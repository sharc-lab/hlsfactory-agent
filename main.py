import argparse
import subprocess
import json
import uuid
from datetime import datetime, timezone
from pathlib import Path

from minisweagent.agents.default import DefaultAgent
from minisweagent.models import get_model
from minisweagent.environments.docker import DockerEnvironment


def build_prompt(repo_url: str) -> str:
    return (
        f"Process the HLS repository at '{repo_url}'.\n"
        "\n"
        "Execute the complete HLSFactory pipeline:\n"
        f"1. Clone the repository from '{repo_url}' into /workspace/repo\n"
        "2. Analyze the repository and identify all HLS designs\n"
        "3. Extract each design into its own folder under /output\n"
        "4. Find or generate testbenches for each design\n"
        "5. Generate documentation for each design\n"
        "6. Compile with clang++ and fix any errors (HLS stubs are at /workspace/stubs)\n"
        "7. Generate TCL synthesis scripts\n"
        "8. Create the final manifest\n"
        "9. Put all design folders in a parent folder named after the repo under /output\n"
        "\n"
        "Work through each stage systematically and process ALL designs found.\n"
        "Start executing immediately — begin by cloning the repository now."
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

    # script_dir = Path(__file__).resolve().parent
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
    agent = DefaultAgent(
        get_model("openrouter/moonshotai/kimi-k2.5"),
        env,
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
          "model": "openrouter/moonshotai/kimi-k2.5",
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


if __name__ == "__main__":
    main()
