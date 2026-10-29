# Zorba command-line image.
# Build (from the repo root):
#   docker build -t zorba -f docker/runtime/Dockerfile .
# Run:
#   docker run --rm zorba -q '1 + 1'
#   docker run --rm -v "$PWD":/work zorba -f -q query.xq

FROM ubuntu:24.04 AS build
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
      build-essential cmake ninja-build ca-certificates pkg-config \
      libicu-dev libxml2-dev libxslt1-dev libcurl4-openssl-dev zlib1g-dev \
      libxerces-c-dev uuid-dev \
 && rm -rf /var/lib/apt/lists/*
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
      libicu74 libxml2 libxslt1.1 libcurl4t64 zlib1g libxerces-c3.2t64 \
      libuuid1 ca-certificates \
 && rm -rf /var/lib/apt/lists/*
COPY --from=build /opt/zorba /opt/zorba
RUN echo /opt/zorba/lib > /etc/ld.so.conf.d/zorba.conf && ldconfig
ENV PATH=/opt/zorba/bin:$PATH
WORKDIR /work
ENTRYPOINT ["zorba"]
CMD ["--help"]
