import argparse
import shutil
import subprocess
from pathlib import Path


def derive_repo_name(url: str) -> str:
    cleaned = url.rstrip('/')
    if cleaned.endswith('.git'):
        cleaned = cleaned[:-4]
    parts = cleaned.split('/')
    return parts[-1] if parts else 'repo'


def main() -> None:
    parser = argparse.ArgumentParser(description="Fetch HLS sources from GitHub and extract designs into HLSDesigns")
    parser.add_argument("github_url", type=str, help="GitHub repository URL")
    parser.add_argument("--github_branch", type=str, default=None, help="Branch to fetch (default: auto-detect)")
    parser.add_argument("--github_subdir", type=str, default=None, help="Subdirectory in repo to use as source")
    parser.add_argument("--name", type=str, default=None, help="Folder name to use under HLSSourceCode (default: repo name)")
    parser.add_argument("--group_name", type=str, default=None, help="Group name under HLSDesigns (default: same as --name)")
    parser.add_argument("--interactive", action="store_true", help="Prompt for names if not provided")
    parser.add_argument("--model_id__extract_top_level_designs", type=str, default="google/gemini-2.5-flash")
    parser.add_argument("--model_id__break_down_hls_design", type=str, default="openai/gpt-5-nano")
    args = parser.parse_args()

    repo_name = derive_repo_name(args.github_url)
    source_name = args.name or (input(f"Name for HLSSourceCode folder [{repo_name}]: ").strip() if args.interactive else "") or repo_name
    group_name = args.group_name or (input(f"Name for HLSDesigns group [{source_name}]: ").strip() if args.interactive else "") or source_name
    branch = args.github_branch
    if args.interactive and branch is None:
        typed = input("Branch to fetch [auto-detect]: ").strip()
        if typed:
            branch = typed

    repo_dest = (Path(__file__).resolve().parents[1] / "HLSSourceCode" / source_name)
    repo_dest.parent.mkdir(parents=True, exist_ok=True)
    if repo_dest.exists():
        shutil.rmtree(repo_dest)

    # Prefer git clone; if it fails, raise and let the user install git
    print(f"Cloning {args.github_url} into {repo_dest} ...")
    cmd = ["git", "clone", "--depth", "1"]
    if branch:
        cmd += ["--branch", branch]
    cmd += [args.github_url, str(repo_dest)]
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        if branch:
            # Retry without explicit branch to auto-detect default
            print("git clone failed with the specified branch, retrying default branch (auto-detect)...")
            if repo_dest.exists():
                shutil.rmtree(repo_dest, ignore_errors=True)
            result = subprocess.run(
                ["git", "clone", "--depth", "1", args.github_url, str(repo_dest)],
                capture_output=True,
                text=True,
            )
        if result.returncode != 0:
            raise RuntimeError(f"git clone failed: {result.stderr or result.stdout}")

    src_dir = repo_dest if args.github_subdir is None else (repo_dest / args.github_subdir)
    out_dir = (Path(__file__).resolve().parents[1] / "HLSDesigns" / group_name)

    if out_dir.exists():
        shutil.rmtree(out_dir)
    out_dir.parent.mkdir(parents=True, exist_ok=True)

    # Invoke the agent
    agent = Path(__file__).resolve().parents[1] / "hlsfactory_agent.py"
    cmd = [
        "python", str(agent), str(src_dir), "--dst_dir", str(out_dir),
        "--model_id__extract_top_level_designs", args.model_id__extract_top_level_designs,
        "--model_id__break_down_hls_design", args.model_id__break_down_hls_design,
    ]
    print("Running:", " ".join(cmd))
    run = subprocess.run(cmd)
    if run.returncode != 0:
        raise SystemExit(run.returncode)


if __name__ == "__main__":
    main()
