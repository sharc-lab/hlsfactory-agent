FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# Tools for the paper-filter agent: PDF reading, repo inspection, API calls. No compilers.
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
      git curl ca-certificates xz-utils jq ripgrep tree \
      poppler-utils python3 python3-pip && \
    rm -rf /var/lib/apt/lists/*

RUN pip3 install --no-cache-dir pymupdf

### Node and pi-coding-agent (same versions as docker_images/Dockerfile) ###
ENV NODE_VERSION=24.15.0

# TARGETARCH is set by BuildKit (amd64 or arm64), so this builds on Intel servers and
# Apple Silicon alike. (docker_images/Dockerfile hardcodes x64.)
ARG TARGETARCH
RUN case "$TARGETARCH" in \
      amd64) NODE_ARCH=x64 ;; \
      arm64) NODE_ARCH=arm64 ;; \
      *) echo "unsupported arch: $TARGETARCH" >&2; exit 1 ;; \
    esac && \
    curl -fsSL https://nodejs.org/dist/v$NODE_VERSION/node-v$NODE_VERSION-linux-$NODE_ARCH.tar.xz \
    | tar -xJ -C /usr/local --strip-components=1

RUN npm install -g @mariozechner/pi-coding-agent

RUN mkdir -p /workspace && chmod 777 /workspace
WORKDIR /workspace
