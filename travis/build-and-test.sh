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
if [ "$ENABLE_COVERAGE" == "On" ]; then
    cmake "$REPO_ROOT" -DENABLE_COVERAGE=On -DCMAKE_INSTALL_LIBDIR=lib
    make -j$(nproc) coverage
else
    cmake "$REPO_ROOT" -DCMAKE_INSTALL_LIBDIR=lib

    # build binaries
    make -j$(nproc)

    # run all unit tests
    ctest -V
fi

# install libappimage
DESTDIR=$BUILD_DIR/libappimage make install

# do integration test
mkdir $BUILD_DIR/client_app_build
pushd $BUILD_DIR/client_app_build
cmake -DCMAKE_PREFIX_PATH=$BUILD_DIR/libappimage/usr/local/lib/cmake/libappimage $REPO_ROOT/tests/client_app/
make
./client_app
