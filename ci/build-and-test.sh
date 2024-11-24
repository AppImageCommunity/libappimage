#! /bin/bash

set -e
set -x

# use RAM disk if possible
if [ "$CI" == "" ] && [ -d /dev/shm ]; then
    TEMP_BASE=/dev/shm
else
    TEMP_BASE=/tmp
fi

BUILD_DIR="$(mktemp -d -p "$TEMP_BASE" libappimage-build-XXXXXX)"

cleanup () {
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
    fi
}

trap cleanup EXIT

# store repo root as variable
REPO_ROOT="$(readlink -f "$(dirname "$(dirname "${BASH_SOURCE[0]}")")")"
OLD_CWD="$(readlink -f "$PWD")"

pushd "$BUILD_DIR"

EXTRA_CMAKE_ARGS=()
if [[ "$LIBAPPIMAGE_SHARED_ONLY" != "" ]]; then
    # shared only builds do not provide any tests
    EXTRA_CMAKE_ARGS+=("-DLIBAPPIMAGE_SHARED_ONLY=ON" "-DBUILD_TESTING=OFF")
fi

# configure build
if [ "$BUILD_TYPE" == "coverage" ]; then
    cmake "$REPO_ROOT" -DCMAKE_INSTALL_LIBDIR=lib -DENABLE_COVERAGE=On "${EXTRA_CMAKE_ARGS[@]}"
    make -j"$(nproc)" coverage
else
    cmake "$REPO_ROOT" -DCMAKE_INSTALL_LIBDIR=lib "${EXTRA_CMAKE_ARGS[@]}"

    # build binaries
    make -j"$(nproc)"

    # run all unit tests
    ctest -V
fi

# install libappimage
DESTDIR="$BUILD_DIR"/libappimage make install

if [[ "$LIBAPPIMAGE_SHARED_ONLY" == "" ]]; then
    # do integration test
    mkdir "$BUILD_DIR"/client_app_build
    pushd "$BUILD_DIR"/client_app_build
    cmake -DCMAKE_PREFIX_PATH="$BUILD_DIR"/libappimage/usr/local/lib/cmake/libappimage "$REPO_ROOT"/tests/client_app/ "${EXTRA_CMAKE_ARGS[@]}"
    make
    ./client_app
fi
