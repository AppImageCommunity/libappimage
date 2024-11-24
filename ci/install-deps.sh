#! /bin/bash

set -euo pipefail

if [[ "${CI:-}" == "" ]]; then
    echo "Caution: this script is supposed to run inside a (disposable) CI environment"
    echo "It will alter a system, and should not be run on workstations or alike"
    echo "You can export CI=1 to prevent this error from being shown again"
    exit 3
fi

set -x

packages=(
    libfuse-dev
    desktop-file-utils
    ca-certificates
    make
    build-essential
    git
    automake
    autoconf
    libtool
    patch
    wget
    vim-common
    desktop-file-utils
    pkg-config
    libarchive-dev
    librsvg2-dev
    librsvg2-bin
    liblzma-dev
    cmake
    lcov
    gcovr
    libboost-dev
)

# make sure installation won't hang on GitHub actions
export DEBIAN_FRONTEND=noninteractive

apt-get update
apt-get -y --no-install-recommends install "${packages[@]}"
