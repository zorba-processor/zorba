# Zorba build environment.
#   baseline: Ubuntu 24.04 (GCC 13, ICU 74, libxml2 2.9) - builds the 2016 tree with few changes
# Build:  docker build -t zorba-builder:baseline docker/builder
#
# Xerces-C is built from the zorba-processor fork (tag pinned to a commit)
# instead of the distribution package: stock Xerces-C <= 3.3.0 rejects valid
# schemas that restrict a union type (see KNOWN_ISSUES.txt). It is built in
# the layer it is installed in, against the image's own ICU and curl, and
# only its headers and libraries are kept, in /usr/local.
ARG BASE=ubuntu:24.04
FROM ${BASE}

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
      build-essential clang cmake ninja-build ccache git ca-certificates pkg-config \
      bison flex libfl-dev \
      libicu-dev libxml2-dev libxslt1-dev libcurl4-openssl-dev zlib1g-dev \
      uuid-dev locales gdb \
 && rm -rf /var/lib/apt/lists/* \
 && for l in en_US de_DE fr_FR; do localedef -i $l -f UTF-8 $l.UTF-8; done

ARG XERCES_REPO=https://github.com/zorba-processor/xerces-c.git
ARG XERCES_TAG=v3.3.0-zorba.1
ARG XERCES_COMMIT=83a6f00a8af3534c98976bee9730fa94dca7c781
RUN git clone --depth 1 --branch "${XERCES_TAG}" "${XERCES_REPO}" /tmp/xerces \
 && test "$(git -C /tmp/xerces rev-parse HEAD)" = "${XERCES_COMMIT}" \
 && cmake -S /tmp/xerces -B /tmp/xerces/build -G Ninja -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_INSTALL_LIBDIR=lib \
      -Dtranscoder=icu -Dnetwork-accessor=curl \
 && cmake --build /tmp/xerces/build \
 && DESTDIR=/tmp/xerces/out cmake --install /tmp/xerces/build \
 && cp -a /tmp/xerces/out/usr/local/include /tmp/xerces/out/usr/local/lib /usr/local/ \
 && rm -rf /tmp/xerces \
 && ldconfig

ENV CCACHE_DIR=/ccache \
    CCACHE_MAXSIZE=20G \
    CMAKE_GENERATOR=Ninja \
    CMAKE_C_COMPILER_LAUNCHER=ccache \
    CMAKE_CXX_COMPILER_LAUNCHER=ccache
WORKDIR /src
