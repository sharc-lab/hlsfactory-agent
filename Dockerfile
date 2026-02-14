# use python as base image
FROM python:3.11-slim

# install git, curl, clang
RUN apt-get update && \
    apt-get install -y git curl clang && \
    rm -rf /var/lib/apt/lists/*

# install opencode
RUN curl -fsSL https://opencode.ai/install | bash
# where opencode is installed
ENV PATH="/root/.opencode/bin:${PATH}"

# set the working directory to /workspace
WORKDIR /workspace

# copy opencode config
COPY opencode.json .

# copy HLS stub headers
COPY hlsfactory-opencode-native/stubs/ ./stubs/

# create output directory
RUN mkdir -p /output
