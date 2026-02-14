import subprocess
import sys
import os
from pathlib import Path
from dotenv import load_dotenv

# ensure repository url is included
if len(sys.argv) < 2:
    print("Usage: python agent_script.py SOURCE_REPO_URL")
    sys.exit()

# get the current directory
script_dir = Path(__file__).parent.resolve()
project_root = script_dir # script is currently in project root

# assign source_repo and output_dir default
source_repo = sys.argv[1]
output_dir = sys.argv[2] if len(sys.argv) > 2 else "./output"

# access the openrouter api key
load_dotenv()
api_key = os.getenv("OPENROUTER_API_KEY")

prompt = f"""Process the HLS repository at {source_repo} and extract all HLS designs to '/output'.

Execute the complete HLSFactory pipeline:
1. Clone the repository from {source_repo} into /workspace/repo
2. Analyze the repository and identify all HLS designs
3. Extract each design into its own folder under /output
4. Find or generate testbenches for each design
5. Generate documentation for each design
6. Compile with clang++ and fix any errors
7. Generate TCL synthesis scripts
8. Create the final manifest
9. Put all the design folders within a parent folder with the name of repository under /output

The HLS stub headers are available at: /workspace/stubs

Work through each stage systematically and process ALL designs found.

IMPORTANT: Start executing immediately. Do not just list steps — run the actual commands. Begin by cloning the repository now."""

# script starts
print("==============================================")
print("HLSFactory OpenCode Native")
print("==============================================")
print("Source Repository: ", source_repo)
print("Output Directory: ", output_dir)
print("==============================================")

#build docker image
print("Building Docker image...")
subprocess.run(["docker-compose", "-f", f"{project_root}/docker-compose.yml", "build"], check=True)

# Run the agent inside the container, no --rm so we can copy files out
print("Starting OpenCode orchestrator...")
subprocess.run(["docker-compose", "-f", f"{project_root}/docker-compose.yml", "run",
                "hls-organizer", "opencode", "run", "-m", "openrouter/moonshotai/kimi-k2.5",
                prompt], check=True)

print("==============================================")
print("Pipeline complete!")
print("==============================================")

# copy results from container to host
result = subprocess.run(["docker", "ps", "-lq"], capture_output=True, text=True)
container_id = result.stdout.strip()
Path(output_dir).mkdir(parents=True, exist_ok=True)
subprocess.run(["docker", "cp", f"{container_id}:/output/.", f"{output_dir}/"], check=True)

# clean up the container
subprocess.run(["docker", "rm", f"{container_id}"])
print(f"Results copied to {output_dir}")