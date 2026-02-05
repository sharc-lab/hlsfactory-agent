import subprocess
import sys
import os
from pathlib import Path


def clone_repo(repo_url: str, dest: Path) -> None:
    """Clone a GitHub repo to the destination directory."""
    subprocess.run(["git", "clone", repo_url, str(dest)], check=True)


def extract_repo_name(repo_url: str) -> str:
    """Extract repository name from GitHub URL."""
    # Handle both https://github.com/user/repo and https://github.com/user/repo.git
    name = repo_url.rstrip("/").split("/")[-1]
    if name.endswith(".git"):
        name = name[:-4]
    return name


def run_opencode(repo_path: Path, output_path: Path, repo_name: str) -> None:
    """Run opencode to extract HLS designs from the repo."""

    prompt = f"""You are inside a Docker container with full bash access and clang installed.

Your task: Extract all HLS (High-Level Synthesis) designs from the repository at {repo_path} and organize them into {output_path}.

Desired output structure:
```
{output_path}/HLS_DESIGNS_{repo_name}/
├── Design_1/
│   ├── SourceCode/
│   ├── TestBench/
│   └── ... (other relevant files)
├── Design_2/
│   ├── SourceCode/
│   ├── TestBench/
│   └── ...
```

Each design folder should contain all files needed to synthesize that HLS design. Separate source code from testbenches.

You have free reign to:
- Explore the repository structure
- Run any bash commands
- Use clang to verify source code compiles (if you choose)
- Create directories and copy/organize files as needed

Go ahead and extract the HLS designs."""

    subprocess.run(["opencode", prompt], cwd=str(repo_path), check=True)


def main():
    if len(sys.argv) != 2:
        print("Usage: python agent_script.py <github-repo-url>")
        sys.exit(1)

    repo_url = sys.argv[1]
    repo_name = extract_repo_name(repo_url)

    repo_path = Path("/workspace/repo")
    output_path = Path("/output")

    print(f"Cloning {repo_url}...")
    clone_repo(repo_url, repo_path)

    print(f"Running opencode to extract HLS designs...")
    run_opencode(repo_path, output_path, repo_name)

    print(f"Done! Check {output_path}/HLS_DESIGNS_{repo_name}/")


if __name__ == "__main__":
    main()
