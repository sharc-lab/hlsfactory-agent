#!/usr/bin/env python3
"""HLSFactory OpenCode Native - Cross-platform Python entry point.

Replaces run.sh (Linux) and run.bat (Windows) with a single script.
Uses Docker to run the OpenCode orchestrator agent.

Usage:
    python run.py SOURCE_REPO [-o OUTPUT_DIR] [--dry-run]
"""

import argparse
import os
import subprocess
import sys
from pathlib import Path


def load_env(env_path: Path) -> None:
    """Load a .env file into os.environ (simple KEY=VALUE parser)."""
    if not env_path.is_file():
        return
    with open(env_path) as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            if "=" not in line:
                continue
            key, _, value = line.partition("=")
            key = key.strip()
            value = value.strip().strip("'\"")
            os.environ.setdefault(key, value)


def build_prompt(source_repo: str) -> str:
    return (
        f"Process the HLS repository at '{source_repo}' and extract all HLS designs to '/output'.\n"
        "\n"
        "Execute the complete HLSFactory pipeline:\n"
        f"1. Clone the repository from '{source_repo}' into /workspace/repo\n"
        "2. Analyze the repository and identify all HLS designs\n"
        "3. Extract each design into its own folder under /output\n"
        "4. Find or generate testbenches for each design\n"
        "5. Generate documentation for each design\n"
        "6. Compile with clang++ and fix any errors\n"
        "7. Generate TCL synthesis scripts\n"
        "8. Create the final manifest\n"
        "9. Put all the design folders within a parent folder with the name of repository under /output\n"
        "\n"
        "The HLS stub headers are available at: /workspace/stubs\n"
        "\n"
        "Work through each stage systematically and process ALL designs found.\n"
        "\n"
        "IMPORTANT: Start executing immediately. Do not just list steps — run the actual commands. "
        "Begin by cloning the repository now."
    )


def run_cmd(cmd: list[str], dry_run: bool = False, **kwargs) -> None:
    """Run a command, streaming output to the terminal."""
    print(f"  > {' '.join(cmd)}")
    if dry_run:
        return
    result = subprocess.run(cmd, **kwargs)
    if result.returncode != 0:
        print(f"Command failed with exit code {result.returncode}", file=sys.stderr)
        sys.exit(result.returncode)


def main() -> None:
    parser = argparse.ArgumentParser(
        description="HLSFactory OpenCode Native — run the HLS pipeline via Docker."
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

    # Paths
    script_dir = Path(__file__).resolve().parent
    project_root = script_dir.parent
    compose_file = project_root / "docker-compose.yml"

    # Load .env
    load_env(project_root / ".env")

    # Build prompt
    prompt = build_prompt(args.source_repo)

    output_dir = Path(args.output_dir)

    # Banner
    print("==============================================")
    print("HLSFactory OpenCode Native")
    print("==============================================")
    print(f"Source Repository: {args.source_repo}")
    print(f"Output Directory:  {output_dir}")
    if args.dry_run:
        print("Mode:              DRY RUN")
    print("==============================================")
    print()

    # Build Docker image
    print("Building Docker image...")
    run_cmd(
        ["docker-compose", "-f", str(compose_file), "build"],
        dry_run=args.dry_run,
    )

    # Run the agent inside the container (no --rm so we can copy files out)
    print("Starting OpenCode orchestrator...")
    print()
    run_cmd(
        [
            "docker-compose", "-f", str(compose_file), "run",
            "hls-organizer",
            "opencode", "run", "-m", "openrouter/moonshotai/kimi-k2.5", prompt,
        ],
        dry_run=args.dry_run,
    )

    print()
    print("==============================================")
    print("Pipeline complete!")
    print("==============================================")

    # Copy results from the container to the host
    if not args.dry_run:
        container_id = subprocess.run(
            ["docker", "ps", "-lq"],
            capture_output=True, text=True, check=True,
        ).stdout.strip()

        if not container_id:
            print("Warning: could not find container ID, skipping copy.", file=sys.stderr)
        else:
            output_dir.mkdir(parents=True, exist_ok=True)
            run_cmd(["docker", "cp", f"{container_id}:/output/.", str(output_dir)])
            run_cmd(["docker", "rm", container_id])
    else:
        print("  > docker ps -lq")
        print(f"  > docker cp <container>:/output/. {output_dir}")
        print("  > docker rm <container>")

    print(f"Results copied to {output_dir}")


if __name__ == "__main__":
    main()
