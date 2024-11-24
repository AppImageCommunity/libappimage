#! /bin/bash

if [[ "${ARCH:-}" == "" ]] || [[ "${RELEASE:-}" == "" ]]; then
    echo "Usage: env ARCH=... RELEASE=... bash $0"
    exit 1
fi

set -euo pipefail

# the other script sources this script, therefore we have to support that use case
if [[ "${BASH_SOURCE[*]}" != "" ]]; then
    this_dir="$(readlink -f "$(dirname "${BASH_SOURCE[0]}")")"
else
    this_dir="$(readlink -f "$(dirname "$0")")"
fi

image=quay.io/appimage/libappimage-build:"$RELEASE"

extra_build_args=()
if [[ "${NO_PULL:-}" == "" ]]; then
    # speed up build by pulling last built image from quay.io and building the docker file using the old image as a base
    docker pull "$image" || true
    extra_build_args=(--cache-from "$image")
fi

# if the image hasn't changed, this should be a no-op
docker build \
    --platform "$platform" \
    --pull \
    --build-arg ARCH \
    --build-arg RELEASE \
    -t "$image" \
    "${extra_build_args[@]}" \
    "$this_dir"

# push built image as cache for future builds to registry
# we can do that immediately once the image has been built successfully; if its definition ever changes it will be
# rebuilt anyway
# credentials shall only be available on (protected) master branch
set +x
if [[ "${DOCKER_USERNAME:-}" != "" ]]; then
    echo "$DOCKER_PASSWORD" | docker login -u "$DOCKER_USERNAME" --password-stdin quay.io
    docker push "$image"
fi
