# Zorba build environment - latest toolchain (rolling: Arch Linux).
# Pin BASE to a dated tag (archlinux:base-YYYYMMDD.0.NNNNNN) for reproducible builds.
# Build:  docker build -t zorba-builder:latest docker/builder-latest
ARG BASE=archlinux:base
FROM ${BASE}

RUN pacman -Syu --noconfirm --needed \
      base-devel gcc clang cmake ninja ccache git pkgconf \
      bison flex python \
      icu libxml2 libxslt curl zlib xerces-c util-linux-libs gdb glibc-locales \
 && pacman -Scc --noconfirm

ENV CCACHE_DIR=/ccache \
    CCACHE_MAXSIZE=20G \
    CMAKE_GENERATOR=Ninja \
    CMAKE_C_COMPILER_LAUNCHER=ccache \
    CMAKE_CXX_COMPILER_LAUNCHER=ccache
WORKDIR /src
