#! /bin/bash

set -e

if [[ "$ARCH" == "" ]]; then
    echo "Usage: env ARCH=... bash $0"
    exit 2
fi

if [[ "$CI" == "" ]]; then
    echo "Caution: this script is supposed to run inside a (disposable) CI environment"
    echo "It will alter a system, and should not be run on workstations or alike"
    echo "You can export CI=1 to prevent this error from being shown again"
    exit 3
fi

case "$ARCH" in
    x86_64|i386|armhf|arm64)
        ;;
    *)
        echo "Error: unsupported architecture: $ARCH"
        exit 4
        ;;
esac

case "$DIST" in
    bionic|focal)
        ;;
    *)
        echo "Error: unsupported distribution: $DIST"
        exit 5
        ;;
esac

set -x

packages=(
    libfuse-dev
    desktop-file-utils
    ca-certificates
    gcc-multilib
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

# install gcc-10 (supports C++17 with std::filesystem properly)
if [[ "$DIST" == "bionic" ]]; then
    apt-get update
    apt-get install --no-install-recommends -y software-properties-common
    add-apt-repository -y ppa:ubuntu-toolchain-r/test
    packages+=(g++-10-multilib)
else
    packages+=(g++-multilib)
fi

# make sure installation won't hang on GitHub actions
export DEBIAN_FRONTEND=noninteractive

apt-get update
apt-get -y --no-install-recommends install "${packages[@]}"

# install more recent CMake version
wget https://artifacts.assassinate-you.net/prebuilt-cmake/continuous/cmake-v3.24.1-ubuntu_"$DIST"-"$ARCH".tar.gz -qO- | \
    tar xz -C/usr/local --strip-components=1

# g++-10 should be used by default
if [[ "$DIST" == "bionic" ]]; then
    update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++-10 9999
fi
