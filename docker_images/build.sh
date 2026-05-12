#!/bin/sh

DIR_SCRIPT=$(dirname $(realpath $0))
cd $DIR_SCRIPT

DOCKER_BUILDKIT=1 docker build --progress=plain -t hlsfactory-agent -f Dockerfile . 2>&1 | tee build.log