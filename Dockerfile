# use python as base image
FROM python:3.11-slim

# install git, curl, clang
RUN apt-get update && \
    apt-get install -y git curl clang && \
    rm -rf /var/lib/apt/lists/*

# RUN pip install uv && \ 
# uv pip compile pyproject.toml -o requirements.txt && \ 
# pip install -r requirements.txt

# install opencode
RUN curl -fsSL https://opencode.ai/install | bash

# sets the working directory to /workspace
WORKDIR /workspace

# copy the agent script
COPY agent_script.py .

# command to run on start up
# CMD ["python", "agent_script.py"]
# create output directory
RUN mkdir -p /output