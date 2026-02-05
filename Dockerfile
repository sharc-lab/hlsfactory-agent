# use python as base image
FROM python:3.11-slim

# updates package list, installs git and curl, deletes cached package lists to save space
RUN apt-get update && \ 
    apt-get install -y git curl && \ 
    rm -rf /var/lib/apt/lists/*

# sets the working directory to /workspace
WORKDIR /workspace

# copies all the libraries
COPY pyproject.toml uv.lock* ./
RUN pip install uv && \ 
    uv pip compile pyproject.toml -o requirements.txt && \ 
    pip install -r requirements.txt

# copies our opencode script - doesn't exist yet
# COPY agent_script.py

# create output directory
RUN mkdir -p /output

# command to run on start up
# CMD ["python", "agent_script.py"]