# these args are available *only* for the FROM call
ARG DOCKER_ARCH
ARG DIST

FROM $DOCKER_ARCH/ubuntu:$DIST

# we need to repeat all args from above which we need during build and runtime to make them available
ARG ARCH
ARG DIST

ENV CI=1

COPY ./install-deps.sh /
# see above, for build time we need to pass the args manually (using ENV does not work)
RUN bash -xe install-deps.sh

# create unprivileged user for non-build-script use of this image
# build-in-docker.sh will likely not use this one, as it enforces the caller's uid inside the container
RUN adduser --system --group build
USER build
