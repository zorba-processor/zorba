# ClusterFuzzLite build image for Zorba's libFuzzer targets (test/fuzz).
# Core build: no Xerces (ZORBA_NO_XMLSCHEMA), no curl (no network access from
# the fuzzers). flex/bison are not needed: the parsers are pregenerated.
FROM gcr.io/oss-fuzz-base/base-builder
# The Ubuntu mirrors drop connections from CI runners now and then: retry each
# download, and fail a dead connection in 30 s instead of the default 120 s.
RUN printf 'Acquire::Retries "5";\nAcquire::http::Timeout "30";\nAcquire::https::Timeout "30";\n' \
      > /etc/apt/apt.conf.d/80-retries
RUN apt-get update && apt-get install -y --no-install-recommends \
      ninja-build pkg-config libicu-dev libxml2-dev uuid-dev zlib1g-dev \
 && rm -rf /var/lib/apt/lists/*
COPY . $SRC/zorba
WORKDIR $SRC/zorba
COPY .clusterfuzzlite/build.sh $SRC/
