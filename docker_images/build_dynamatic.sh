#!/bin/sh
# Build the Dynamatic image as the invoking user so files it writes into bind mounts stay ours.
set -e
cd "$(dirname "$0")"
exec docker build -f Dockerfile.dynamatic --build-arg UID="$(id -u)" --build-arg GID="$(id -g)" \
    --build-arg UNAME=dyn -t hlsfactory/dynamatic:latest .
