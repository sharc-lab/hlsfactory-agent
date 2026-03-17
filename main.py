import argparse
import subprocess
import sys
import json
import uuid
import os
from datetime import datetime, timezone
from pathlib import Path

from minisweagent.agents.default import DefaultAgent
from minisweagent.models.openrouter_textbased_model import OpenRouterTextbasedModel
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
        "## Pipeline Steps\n"
        "\n"
        f"### Step 1: Clone the repository\n"
        f"```\n"
        f"git clone '{repo_url}' /workspace/repo\n"
        f"```\n"
        "\n"
        "### Step 2: Analyze the repository\n"
        "Explore the cloned repo and identify ALL HLS designs. Each design is typically:\n"
        "- A C/C++ source file (or set of files) containing a top-level HLS function\n"
        "- May use frameworks: Vitis HLS, Vivado HLS, TAPA, Intel HLS, or LegUp\n"
        "\n"
        "**RTL detection:** If the repository contains ONLY Verilog (.v) / SystemVerilog (.sv) / VHDL (.vhd/.vhdl) files\n"
        "with NO C/C++ HLS source code, then this is a pure RTL repository. In that case:\n"
        "- Still extract each RTL module as a design folder under /output\n"
        "- Copy the RTL sources into the design folder\n"
        "- Skip compilation (no clang needed for RTL)\n"
        "- Note 'rtl_only: true' in the manifest\n"
        "\n"
        "### Step 3: Extract each design\n"
        "For each design found, create a folder under /output/<repo_name>/<design_name>/ containing:\n"
        "- All source files for that design\n"
        "- Any shared or common headers the design depends on (e.g. files in common/, include/, or parent directories referenced via relative #include paths) — copy these into the design folder so it is fully self-contained\n"
        "- A README.md describing the design\n"
        "\n"
        "### Step 4: Find or generate testbenches\n"
        "For each design:\n"
        "- If a testbench already exists in the repo, copy it into the design folder\n"
        "- If not, generate a comprehensive testbench that actually instantiates and calls the top-level function\n"
        "  with sample inputs and checks outputs — NOT a placeholder with TODO comments. Also don't generate a testbench that only returns \n"
        "0. Analyze the design's components and purpose to write a testbench that tests all core functionality."
        "\n"
        "### Step 5: Compile each design\n"
        "Use this exact compilation command for each C/C++ source file:\n"
        "```\n"
        "clang++ -std=c++17 -I/workspace/stubs -I<design_dir> -w -fsyntax-only <file.cpp>\n"
        "```\n"
        "- `-I/workspace/stubs` provides stub headers for ap_int.h, ap_fixed.h, hls_stream.h, tapa.h, hls_math.h, ap_axi_sdata.h, and CL/opencl.h\n"
        "- `-w` suppresses warnings (we only care about errors)\n"
        "- `-fsyntax-only` checks syntax without generating output files\n"
        "- If compilation fails due to missing headers, check the original repo for shared/common header files and copy them into the design folder, then retry\n"
        "- Record compilation results (pass/fail with error messages) in compile_log.txt inside each design folder\n"
        "\n"
        "**TAPA designs:** The stub header `/workspace/stubs/tapa.h` is available. TAPA designs use\n"
        "`#include <tapa.h>` and types like `tapa::mmap<T>`, `tapa::stream<T>`, `tapa::task`.\n"
        "These designs SHOULD be compiled with the stubs — do NOT skip them.\n"
        "\n"
        "**If compilation fails**, try to fix obvious include path issues before recording as failed.\n"
        "For example, add `-I` flags for subdirectories that contain referenced headers.\n"
        "\n"
        "### Step 6: Generate TCL synthesis scripts\n"
        "For each design, generate a TCL script (synth.tcl) for Vivado HLS / Vitis HLS:\n"
        "- `set_top` MUST match the actual top-level function name found in the source code\n"
        "- `set_part` should match the target FPGA device from the repo if specified, otherwise use `xcu250-figd2104-2L-e`\n"
        "- Include `add_files` for all source files and `add_files -tb` for testbenches\n"
        "- Include `open_solution`, `csynth_design`, and `exit` commands\n"
        "\n"
        "### Step 7: Create manifest.json\n"
        "Create a `/output/<repo_name>/manifest.json` with this exact schema:\n"
        "```json\n"
        "{\n"
        '  "repo_name": "<name>",\n'
        '  "repo_url": "<url>",\n'
        '  "framework": "<vitis_hls|vivado_hls|tapa|intel_hls|legup|unknown>",\n'
        '  "total_designs": <number>,\n'
        '  "designs": [\n'
        "    {\n"
        '      "name": "<design_name>",\n'
        '      "top_function": "<function_name>",\n'
        '      "compile_status": "<pass|fail|skip>",\n'
        '      "has_testbench": <true|false>,\n'
        '      "has_tcl": <true|false>,\n'
        '      "source_files": ["<file1.cpp>", ...]\n'
        "    }\n"
        "  ]\n"
        "}\n"
        "```\n"
        "\n"
        "### Step 8: Final organization\n"
        "Ensure all design folders are under `/output/<repo_name>/` with the manifest at the top level.\n"
        "Run `find /output -type f | head -50` to verify the output structure.\n"
        "\n"
        "## Important Rules\n"
        "- Process ALL designs found. Do not skip any design.\n"
        "- Start executing immediately. Do not just list steps — run the actual commands.\n"
        "- Begin by cloning the repository now.\n"
        "- Use `tree` or `find` to explore directories — both are available.\n"
        "\n"
        "## Pre-Submission Checklist (REQUIRED before submitting)\n"
        "Before issuing the submit command, you MUST verify all of the following:\n"
        "1. Run `ls /output/<repo_name>/ | wc -l` to count design folders — call this N\n"
        "2. Run `cat /output/<repo_name>/manifest.json` — the `designs` array must have exactly N entries, one per design folder\n"
        "3. Each entry in `designs` must correspond to exactly ONE design folder — a single entry covering the whole repo (e.g. 'all_design') is NOT valid\n"
        "4. Every design folder must contain: at least one .cpp source file, a testbench.cpp, a synth.tcl, and a compile_log.txt\n"
        "5. If the designs array has fewer entries than folders, you have NOT finished — go back and process the remaining designs\n"
        "Do NOT submit until all checks pass.\n"
    )


def compute_benchmark_stats(output_dir: Path, repo_name: str) -> dict:
    """Scan the output directory and compute quality metrics."""
    stats = {
        "designs_found": 0,
        "compile_pass": 0,
        "compile_fail": 0,
        "compile_skip": 0,
        "has_manifest": False,
        "manifest_valid_json": False,
        "designs_with_testbench": 0,
        "designs_with_tcl": 0,
    }

    repo_dir = output_dir / repo_name
    if not repo_dir.is_dir():
        # Try to find any subdirectory that might be the repo
        subdirs = [d for d in output_dir.iterdir() if d.is_dir()]
        if len(subdirs) == 1:
            repo_dir = subdirs[0]
        else:
            return stats

    # Check manifest
    manifest_path = repo_dir / "manifest.json"
    if manifest_path.is_file():
        stats["has_manifest"] = True
        try:
            manifest = json.loads(manifest_path.read_text())
            stats["manifest_valid_json"] = True
            if "designs" in manifest:
                for d in manifest["designs"]:
                    status = d.get("compile_status", "skip")
                    if status == "pass":
                        stats["compile_pass"] += 1
                    elif status == "fail":
                        stats["compile_fail"] += 1
                    else:
                        stats["compile_skip"] += 1
        except (json.JSONDecodeError, KeyError):
            pass

    # Count design folders and check for testbenches/tcl
    for item in repo_dir.iterdir():
        if item.is_dir():
            stats["designs_found"] += 1
            # Check for testbench files
            tb_patterns = ["*testbench*", "*tb_*", "*_tb.*", "*test_*"]
            has_tb = any(item.glob(p) for p in tb_patterns)
            if has_tb:
                stats["designs_with_testbench"] += 1
            # Check for TCL scripts
            if list(item.glob("*.tcl")):
                stats["designs_with_tcl"] += 1

    return stats


def count_format_errors(traj_path: Path) -> int:
    """Count format error messages in the trajectory file."""
    count = 0
    if traj_path.is_file():
        try:
            traj = json.loads(traj_path.read_text())
            for entry in traj.get("trajectory", []):
                content = str(entry.get("content", ""))
                if "format_error" in content.lower() or "No tool calls found" in content:
                    count += 1
        except (json.JSONDecodeError, KeyError):
            pass
    return count


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
        image="hlsfactory",
        cwd="/workspace",
        forward_env=["OPENROUTER_API_KEY"],
        timeout=120,
        run_args=[],
    )

    # 3. instantiate the agent
    agent_config = get_config_from_spec("default")["agent"]
    agent = DefaultAgent(
        OpenRouterTextbasedModel(model_name="openai/gpt-oss-120b"),
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

    # 6. copy /output from the container to the host
    container_id = env.container_id
    if not container_id:
        print("Warning: could not find container ID, skipping output copy.", file=sys.stderr)
    else:
        subprocess.run(["docker", "cp", f"{container_id}:/output/.", str(output_dir)], check=True)
        subprocess.run(["docker", "rm", "-f", container_id], check=True)
        print(f"HLS results copied to {output_dir}")
        # Prevent double-cleanup since we already removed the container
        env.container_id = None

    # 7. compute benchmark stats from output
    quality_stats = compute_benchmark_stats(output_dir, repo_name)
    format_errors = count_format_errors(traj_path)

    # 8. our benchmark json
    benchmark = {
          "run_id": run_id,
          "timestamp": datetime.now(timezone.utc).isoformat(),
          "repo_url": repo_url,
          "model": "openai/gpt-oss-120b",
          "exit_status": result.get("exit_status"),
          "total_cost_usd": agent.cost,
          "total_api_calls": agent.n_calls,
          "format_errors": format_errors,
          "traj_file": traj_path.name,
          "quality": quality_stats,
    }

    benchmark_path = output_dir / f"{repo_name}_{run_id}_benchmark.json"
    benchmark_path.write_text(json.dumps(benchmark, indent=2))

    print(f"Trajectory: {traj_path}")
    print(f"Benchmark:  {benchmark_path}")
    print(f"Designs found: {quality_stats['designs_found']}, "
          f"Compile pass/fail/skip: {quality_stats['compile_pass']}/{quality_stats['compile_fail']}/{quality_stats['compile_skip']}, "
          f"Format errors: {format_errors}")


if __name__ == "__main__":
    main()
