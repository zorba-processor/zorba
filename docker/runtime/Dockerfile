# Zorba command-line image.
# Build (from the repo root):
#   docker build -t zorba -f docker/runtime/Dockerfile .
# Run:
#   docker run --rm zorba -r -q '1 + 1'
#   echo 'for $i in 1 to 3 return $i * $i' > query.xq
#   docker run --rm -v "$PWD":/work zorba -r -f -q query.xq
# MCP server over a folder of XML, JSON, CSV and HTML files (stdio):
#   docker run -i --rm -v "$PWD":/data:ro zorba zorba-mcp --root /data

FROM ubuntu:24.04 AS build
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
      build-essential cmake ninja-build meson python3 ca-certificates pkg-config \
      libcurl4-openssl-dev zlib1g-dev \
      uuid-dev git \
 && rm -rf /var/lib/apt/lists/*
# ICU and libxml2 from the patched zorba-processor forks, as in docker/builder
# (keep the tags and commits in sync): ICU's regular expression compiler
# crashes and libxml2's canonicalization drops content when an allocation
# fails (see doc/KNOWN_ISSUES.txt). Ubuntu's ICU and libxml2 are installed
# neither in this stage nor in the image, so only the patched copy is loaded.
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
RUN git clone --depth 1 --branch "${LIBXML2_TAG}" "${LIBXML2_REPO}" /tmp/libxml2 \
 && test "$(git -C /tmp/libxml2 rev-parse HEAD)" = "${LIBXML2_COMMIT}" \
 && meson setup /tmp/libxml2/build /tmp/libxml2 --prefix=/usr/local --libdir=lib \
      --buildtype=release --auto-features=enabled \
      -Ddocs=disabled -Dpython=disabled -Dreadline=disabled -Dhistory=disabled \
      -Dicu=enabled -Dlegacy=enabled \
 && meson install -C /tmp/libxml2/build \
 && rm -rf /tmp/libxml2 \
 && ldconfig
# Xerces-C from the patched zorba-processor fork, as in docker/builder (keep
# the tag and commit in sync): stock Xerces-C <= 3.3.0 rejects valid schemas
# that restrict a union type (see doc/KNOWN_ISSUES.txt).
ARG XERCES_REPO=https://github.com/zorba-processor/xerces-c.git
ARG XERCES_TAG=v3.3.0-zorba.5
ARG XERCES_COMMIT=c35c28f192a28d9ae1ddde635294f43536dccffd
RUN git clone --depth 1 --branch "${XERCES_TAG}" "${XERCES_REPO}" /tmp/xerces \
 && test "$(git -C /tmp/xerces rev-parse HEAD)" = "${XERCES_COMMIT}" \
 && cmake -S /tmp/xerces -B /tmp/xerces/build -G Ninja -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_INSTALL_LIBDIR=lib \
      -Dtranscoder=icu -Dnetwork-accessor=curl \
 && cmake --build /tmp/xerces/build --target xerces-c \
 && cmake --install /tmp/xerces/build/src \
 && rm -rf /tmp/xerces \
 && ldconfig
# The shared libraries the image ships, with their soname links, and the
# checksums scripts/check-patched-libs.sh verifies.
RUN mkdir /opt/libs \
 && cp -P /usr/local/lib/libicuuc.so.* /usr/local/lib/libicui18n.so.* \
      /usr/local/lib/libicudata.so.* /usr/local/lib/libxml2.so.* \
      /usr/local/lib/libxerces-c-3.3.so /opt/libs/ \
 && for l in libicuuc libicui18n libicudata libxml2; do readlink -f /usr/local/lib/$l.so; done \
      | xargs sha256sum > /opt/zorba-patched-libs.sha256
COPY . /src
WORKDIR /src
RUN cmake --preset full-release \
      -DCMAKE_INSTALL_PREFIX=/opt/zorba -DZORBA_WITH_UNIT_TESTS=OFF \
 && cmake --build /src/build/full-release \
 && cmake --install /src/build/full-release \
 && rm -f /opt/zorba/bin/testdriver*

FROM ubuntu:24.04
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
      libcurl4t64 zlib1g \
      libuuid1 ca-certificates \
 && rm -rf /var/lib/apt/lists/*
COPY --from=build /opt/libs/ /usr/local/lib/
COPY --from=build /opt/zorba-patched-libs.sha256 /usr/local/share/zorba-patched-libs.sha256
COPY --from=build /opt/zorba /opt/zorba
RUN echo /opt/zorba/lib > /etc/ld.so.conf.d/zorba.conf && ldconfig
ENV PATH=/opt/zorba/bin:$PATH
COPY docker/runtime/entrypoint.sh /usr/local/bin/zorba-entrypoint
WORKDIR /work
ENTRYPOINT ["zorba-entrypoint"]
CMD ["--help"]
