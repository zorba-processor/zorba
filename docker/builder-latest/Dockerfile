# Zorba build environment - latest toolchain (rolling: Arch Linux).
# Pin BASE to a dated tag (archlinux:base-YYYYMMDD.0.NNNNNN) for reproducible builds.
# Build:  docker build -t zorba-builder:latest docker/builder-latest
#
# Xerces-C is built from the zorba-processor fork (tag pinned to a commit)
# instead of the distribution package: stock Xerces-C <= 3.3.0 rejects valid
# schemas that restrict a union type (see doc/KNOWN_ISSUES.txt). It is built in
# the layer it is installed in, against the image's own ICU and curl (Arch
# bumps ICU's soname often), and only its headers and libraries are kept,
# in /usr/local.
ARG BASE=archlinux:base
FROM ${BASE}

RUN pacman -Syu --noconfirm --needed \
      base-devel gcc clang cmake ninja ccache git pkgconf \
      bison flex python \
      icu libxml2 libxslt curl zlib util-linux-libs gdb glibc-locales \
 && pacman -Scc --noconfirm

ARG XERCES_REPO=https://github.com/zorba-processor/xerces-c.git
ARG XERCES_TAG=v3.3.0-zorba.5
ARG XERCES_COMMIT=c35c28f192a28d9ae1ddde635294f43536dccffd
# Arch's dynamic loader does not search /usr/local/lib by default.
RUN git clone --depth 1 --branch "${XERCES_TAG}" "${XERCES_REPO}" /tmp/xerces \
 && test "$(git -C /tmp/xerces rev-parse HEAD)" = "${XERCES_COMMIT}" \
 && cmake -S /tmp/xerces -B /tmp/xerces/build -G Ninja -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_INSTALL_LIBDIR=lib \
      -Dtranscoder=icu -Dnetwork-accessor=curl \
 && cmake --build /tmp/xerces/build \
 && DESTDIR=/tmp/xerces/out cmake --install /tmp/xerces/build \
 && cp -a /tmp/xerces/out/usr/local/include /tmp/xerces/out/usr/local/lib /usr/local/ \
 && rm -rf /tmp/xerces \
 && echo /usr/local/lib > /etc/ld.so.conf.d/usr-local.conf \
 && ldconfig

ENV CCACHE_DIR=/ccache \
    CCACHE_MAXSIZE=20G \
    CCACHE_SLOPPINESS=pch_defines,time_macros \
    CCACHE_DEPEND=1 \
    CMAKE_GENERATOR=Ninja \
    CMAKE_C_COMPILER_LAUNCHER=ccache \
    CMAKE_CXX_COMPILER_LAUNCHER=ccache
WORKDIR /src
