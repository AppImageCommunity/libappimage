#! /bin/bash

set -e
set -x

if [ "$ARCH" == "" ]; then
    echo "Usage: env ARCH=... [DOCKER_IMAGE=...] bash $0"
    exit 2
fi

# build libappimage, and run unit tests
if [ "$DOCKER_IMAGE" != "" ]; then
    docker run --rm \
        --cap-add SYS_ADMIN \
        --device /dev/fuse:mrw \
        -e ARCH -e CI=1 \
        -e CI=1 \
        -i \
        -v "${PWD}":/libappimage \
        "$DOCKER_IMAGE" \
        /bin/bash -xc "cd /libappimage && ci/build-and-test.sh"
else
    exec "$(readlink -f "$(dirname "$0")")"/build-and-test.sh
fi
