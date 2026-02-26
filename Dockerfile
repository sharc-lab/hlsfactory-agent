# use python as base image
FROM python:3.11-slim

# install git, curl, clang
RUN apt-get update && \
    apt-get install -y git curl clang && \
    rm -rf /var/lib/apt/lists/*

# sets the working directory to /workspace
WORKDIR /workspace

# copy HLS stubs
COPY hlsfactory-opencode-native/stubs /workspace/stubs

# command to run on start up
# CMD ["python", "agent_script.py"]
# create output directory
RUN mkdir -p /output