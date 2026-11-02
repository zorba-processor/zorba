# Zorba command-line image.
# Build (from the repo root):
#   docker build -t zorba -f docker/runtime/Dockerfile .
# Run:
#   docker run --rm zorba -q '1 + 1'
#   docker run --rm -v "$PWD":/work zorba -f -q query.xq
# MCP server over a folder of XML, JSON, CSV and HTML files (stdio):
#   docker run -i --rm -v "$PWD":/data:ro zorba zorba-mcp --root /data

FROM ubuntu:24.04 AS build
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
      build-essential cmake ninja-build ca-certificates pkg-config \
      libicu-dev libxml2-dev libxslt1-dev libcurl4-openssl-dev zlib1g-dev \
      uuid-dev git \
 && rm -rf /var/lib/apt/lists/*
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
      libicu74 libxml2 libxslt1.1 libcurl4t64 zlib1g \
      libuuid1 ca-certificates \
 && rm -rf /var/lib/apt/lists/*
COPY --from=build /usr/local/lib/libxerces-c-3.3.so /usr/local/lib/
COPY --from=build /opt/zorba /opt/zorba
RUN echo /opt/zorba/lib > /etc/ld.so.conf.d/zorba.conf && ldconfig
ENV PATH=/opt/zorba/bin:$PATH
COPY docker/runtime/entrypoint.sh /usr/local/bin/zorba-entrypoint
WORKDIR /work
ENTRYPOINT ["zorba-entrypoint"]
CMD ["--help"]
