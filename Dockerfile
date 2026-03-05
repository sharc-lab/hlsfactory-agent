# use python as base image
FROM python:3.11-slim

# install git, curl, clang, tree, make, cmake
RUN apt-get update && \
    apt-get install -y git curl clang tree make cmake && \
    rm -rf /var/lib/apt/lists/*

# install numpy for repos with code generators
RUN pip install --no-cache-dir numpy

# sets the working directory to /workspace
WORKDIR /workspace

# copy HLS stubs
COPY stubs /workspace/stubs

# create output directory
RUN mkdir -p /output
