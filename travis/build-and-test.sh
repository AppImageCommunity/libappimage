#! /bin/bash

set -e
set -x

# use RAM disk if possible
if [ "$CI" == "" ] && [ -d /dev/shm ]; then
    TEMP_BASE=/dev/shm
else
    TEMP_BASE=/tmp
fi

BUILD_DIR=$(mktemp -d -p "$TEMP_BASE" AppImageUpdate-build-XXXXXX)

cleanup () {
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
    fi
}

trap cleanup EXIT

# store repo root as variable
REPO_ROOT=$(readlink -f $(dirname $(dirname $0)))
OLD_CWD=$(readlink -f .)

pushd "$BUILD_DIR"

# configure build
if [ "$BUILD_COVERAGE_TESTS" == "" ]; then
    cmake "$REPO_ROOT"
else
    cmake "$REPO_ROOT" -DBUILD_COVERAGE_TESTS=On
fi

# build binaries
make -j$(nproc)

# run all unit tests
ctest -V

# run coverage tests
if [ "$BUILD_COVERAGE_TESTS" == "On" ]; then
    make coverage_tests
fi
