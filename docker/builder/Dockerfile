# Zorba build environment.
#   baseline: Ubuntu 24.04 (GCC 13) - builds the 2016 tree with few changes
# Build:  docker build -t zorba-builder:baseline docker/builder
#
# ICU and libxml2 are built from the zorba-processor forks (tags pinned to
# commits; the same tags as docker/builder-latest and docker/runtime, keep
# the three in sync) instead of Ubuntu's ICU 74 and libxml2 2.9: ICU's
# regular expression compiler crashes and libxml2's canonicalization drops
# content when an allocation fails (see doc/KNOWN_ISSUES.txt). They are
# newer releases with other sonames than Ubuntu's, installed in /usr/local
# with libxslt built against them, and Ubuntu's development packages are
# not installed, so Zorba, Xerces-C and libxslt load only the patched copy
# (Ubuntu's own libraries may still be present for other packages, such as
# clang). scripts/check-patched-libs.sh checks the result against
# /usr/local/share/zorba-patched-libs.sha256.
#
# Xerces-C is built from the zorba-processor fork (tag pinned to a commit)
# instead of the distribution package: stock Xerces-C <= 3.3.0 rejects valid
# schemas that restrict a union type (see doc/KNOWN_ISSUES.txt). It is built in
# the layer it is installed in, against the image's own ICU and curl, and
# only its headers and libraries are kept, in /usr/local.
ARG BASE=ubuntu:24.04
FROM ${BASE}

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
      build-essential clang cmake ninja-build meson ccache git ca-certificates pkg-config \
      bison flex libfl-dev python3 \
      libcurl4-openssl-dev zlib1g-dev \
      uuid-dev locales gdb \
 && rm -rf /var/lib/apt/lists/* \
 && for l in en_US de_DE fr_FR; do localedef -i $l -f UTF-8 $l.UTF-8; done

ARG ICU_REPO=https://github.com/zorba-processor/icu.git
ARG ICU_TAG=release-78.3-zorba.1
ARG ICU_COMMIT=2ed410f1d6bf3ff218985389538ddad641979af3
RUN git clone --depth 1 --filter=blob:none --sparse --branch "${ICU_TAG}" "${ICU_REPO}" /tmp/icu \
 && test "$(git -C /tmp/icu rev-parse HEAD)" = "${ICU_COMMIT}" \
 && git -C /tmp/icu sparse-checkout set icu4c \
 && cd /tmp/icu/icu4c/source \
 && ./configure --prefix=/usr/local --disable-samples --disable-tests \
 && make -j"$(nproc)" \
 && make install \
 && cd / && rm -rf /tmp/icu \
 && ldconfig

ARG LIBXML2_REPO=https://github.com/zorba-processor/libxml2.git
ARG LIBXML2_TAG=v2.15.4-zorba.1
ARG LIBXML2_COMMIT=e66d697e3a9a855084f3d9243bbb0a8695994cf2
ARG LIBXSLT_REPO=https://github.com/GNOME/libxslt.git
ARG LIBXSLT_TAG=v1.1.45
ARG LIBXSLT_COMMIT=35323d6a15f6e63c9919ddbc0abe64c90a0dd88a
RUN export CCACHE_DISABLE=1 \
 && git clone --depth 1 --branch "${LIBXML2_TAG}" "${LIBXML2_REPO}" /tmp/libxml2 \
 && test "$(git -C /tmp/libxml2 rev-parse HEAD)" = "${LIBXML2_COMMIT}" \
 && meson setup /tmp/libxml2/build /tmp/libxml2 --prefix=/usr/local --libdir=lib \
      --buildtype=release --auto-features=enabled \
      -Ddocs=disabled -Dpython=disabled -Dreadline=disabled -Dhistory=disabled \
      -Dicu=enabled -Dlegacy=enabled \
 && meson install -C /tmp/libxml2/build \
 && rm -rf /tmp/libxml2 \
 && ldconfig \
 && git clone --depth 1 --branch "${LIBXSLT_TAG}" "${LIBXSLT_REPO}" /tmp/libxslt \
 && test "$(git -C /tmp/libxslt rev-parse HEAD)" = "${LIBXSLT_COMMIT}" \
 && cmake -S /tmp/libxslt -B /tmp/libxslt/build -G Ninja -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_INSTALL_LIBDIR=lib \
      -DLIBXSLT_WITH_PYTHON=OFF -DLIBXSLT_WITH_TESTS=OFF \
 && cmake --build /tmp/libxslt/build \
 && cmake --install /tmp/libxslt/build \
 && rm -rf /tmp/libxslt \
 && ldconfig \
 && for l in libicuuc libicui18n libicudata libxml2; do readlink -f /usr/local/lib/$l.so; done \
      | xargs sha256sum > /usr/local/share/zorba-patched-libs.sha256

ARG XERCES_REPO=https://github.com/zorba-processor/xerces-c.git
ARG XERCES_TAG=v3.3.0-zorba.5
ARG XERCES_COMMIT=c35c28f192a28d9ae1ddde635294f43536dccffd
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
    CCACHE_SLOPPINESS=pch_defines,time_macros \
    CCACHE_DEPEND=1 \
    CMAKE_GENERATOR=Ninja \
    CMAKE_C_COMPILER_LAUNCHER=ccache \
    CMAKE_CXX_COMPILER_LAUNCHER=ccache
WORKDIR /src
