# Zorba build environment.
#   baseline: Ubuntu 24.04 (GCC 13, ICU 74, libxml2 2.9) - builds the 2016 tree with few changes
# Build:  docker build -t zorba-builder:baseline docker/builder
ARG BASE=ubuntu:24.04
FROM ${BASE}

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
      build-essential clang cmake ninja-build ccache git ca-certificates pkg-config \
      bison flex libfl-dev \
      libicu-dev libxml2-dev libxslt1-dev libcurl4-openssl-dev zlib1g-dev \
      libxerces-c-dev uuid-dev locales gdb \
 && rm -rf /var/lib/apt/lists/* \
 && for l in en_US de_DE fr_FR; do localedef -i $l -f UTF-8 $l.UTF-8; done

ENV CCACHE_DIR=/ccache \
    CCACHE_MAXSIZE=20G \
    CCACHE_SLOPPINESS=pch_defines,time_macros \
    CCACHE_DEPEND=1 \
    CMAKE_GENERATOR=Ninja \
    CMAKE_C_COMPILER_LAUNCHER=ccache \
    CMAKE_CXX_COMPILER_LAUNCHER=ccache
WORKDIR /src
