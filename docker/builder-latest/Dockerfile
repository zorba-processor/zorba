# Zorba build environment - latest toolchain (rolling: Arch Linux).
# Pin BASE to a dated tag (archlinux:base-YYYYMMDD.0.NNNNNN) for reproducible builds.
# Build:  docker build -t zorba-builder:latest docker/builder-latest
#
# ICU and libxml2 are built from the zorba-processor forks (tags pinned to
# commits) at exactly the versions of Arch's packages and installed over
# them, so every library in the image that links ICU or libxml2 (libxml2,
# libxslt, Xerces-C, Zorba) loads the one patched copy: ICU's regular
# expression compiler crashes and libxml2's canonicalization drops content
# when an allocation fails (see doc/KNOWN_ISSUES.txt). The build stops when
# Arch's version differs from the fork's; move the fork to the new version
# first. scripts/check-patched-libs.sh checks the result against
# /usr/local/share/zorba-patched-libs.sha256.
#
# Xerces-C is built from the zorba-processor fork (tag pinned to a commit)
# instead of the distribution package: stock Xerces-C <= 3.3.0 rejects valid
# schemas that restrict a union type (see doc/KNOWN_ISSUES.txt). It is built in
# the layer it is installed in, against the image's own ICU and curl (Arch
# bumps ICU's soname often), and only its headers and libraries are kept,
# in /usr/local.
#
# The macOS CI job uses Homebrew's stock ICU, libxml2 and Xerces-C.
ARG BASE=archlinux:base
FROM ${BASE}

RUN pacman -Syu --noconfirm --needed \
      base-devel gcc clang cmake ninja meson ccache git pkgconf \
      bison flex python \
      icu libxml2 libxslt curl zlib util-linux-libs gdb glibc-locales \
 && pacman -Scc --noconfirm

ARG ICU_REPO=https://github.com/zorba-processor/icu.git
ARG ICU_VERSION=78.3
ARG ICU_TAG=release-78.3-zorba.1
ARG ICU_COMMIT=2ed410f1d6bf3ff218985389538ddad641979af3
RUN test "$(pacman -Q icu | cut -d' ' -f2 | cut -d- -f1)" = "${ICU_VERSION}" \
 && git clone --depth 1 --filter=blob:none --sparse --branch "${ICU_TAG}" "${ICU_REPO}" /tmp/icu \
 && test "$(git -C /tmp/icu rev-parse HEAD)" = "${ICU_COMMIT}" \
 && git -C /tmp/icu sparse-checkout set icu4c \
 && cd /tmp/icu/icu4c/source \
 && ./configure --prefix=/usr --sysconfdir=/etc --mandir=/usr/share/man \
      --sbindir=/usr/bin --disable-samples --disable-tests \
 && make -j"$(nproc)" \
 && make install \
 && cd / && rm -rf /tmp/icu

ARG LIBXML2_REPO=https://github.com/zorba-processor/libxml2.git
ARG LIBXML2_VERSION=2.15.4
ARG LIBXML2_TAG=v2.15.4-zorba.1
ARG LIBXML2_COMMIT=e66d697e3a9a855084f3d9243bbb0a8695994cf2
RUN test "$(pacman -Q libxml2 | cut -d' ' -f2 | cut -d- -f1)" = "${LIBXML2_VERSION}" \
 && git clone --depth 1 --branch "${LIBXML2_TAG}" "${LIBXML2_REPO}" /tmp/libxml2 \
 && test "$(git -C /tmp/libxml2 rev-parse HEAD)" = "${LIBXML2_COMMIT}" \
 && export CCACHE_DISABLE=1 \
 && meson setup /tmp/libxml2/build /tmp/libxml2 --prefix=/usr --libexecdir=lib \
      --sbindir=bin --buildtype=release --auto-features=enabled \
      -Ddocs=disabled -Dpython=disabled -Dreadline=disabled -Dhistory=disabled \
      -Dicu=enabled -Dlegacy=enabled \
 && meson install -C /tmp/libxml2/build \
 && rm -rf /tmp/libxml2 \
 && for l in libicuuc libicui18n libicudata libxml2; do readlink -f /usr/lib/$l.so; done \
      | xargs sha256sum > /usr/local/share/zorba-patched-libs.sha256

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
