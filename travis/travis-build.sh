#! /bin/bash

set -e
set -x

# build libappimage, and run unit tests
if [ "$DOCKER_IMAGE" != "" ]; then
    docker run --rm \
        --cap-add SYS_ADMIN \
        --device /dev/fuse:mrw \
        -e ARCH -e TRAVIS -e TRAVIS_BUILD_NUMBER \
        -e CI=1 \
        -i \
        -v "${PWD}":/libappimage \
        "$DOCKER_IMAGE" \
        /bin/bash -xc "cd /libappimage && travis/build-and-test.sh"
else
    exec $(readlink -f $(dirname "$0"))/build-and-test.sh
fi
