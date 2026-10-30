<p align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="doc/logo/zorba-logo-dark.svg">
    <img src="doc/logo/zorba-logo.svg" alt="Zorba" height="64">
  </picture>
</p>

Zorba is an embeddable XQuery and JSONiq processor written in C++. It
implements XQuery 3.0, XQuery Update Facility, XQuery Full Text, XQuery
Scripting and JSONiq, with XML Schema validation and an in-memory store with
collections and indexes.

[![CI](https://github.com/zorba-processor/zorba/actions/workflows/ci.yml/badge.svg)](https://github.com/zorba-processor/zorba/actions/workflows/ci.yml)

## Documentation

Tutorial and reference for XQuery, JSONiq and Zorba, with examples verified
against the current build: https://zorba-processor.github.io/zorba/

## Docker

```bash
docker run --rm ghcr.io/zorba-processor/zorba:edge -q '1 + 1'
docker run --rm -v "$PWD":/work ghcr.io/zorba-processor/zorba:edge -f -q query.xq
```

`edge` is the latest master build; releases are tagged by version, and
`latest` is the newest stable release. See [docker/README.md](docker/README.md)
for all image tags.

## Building

Requirements: a C++23 compiler (GCC 13+, Clang 17+ or Apple Clang 16+),
CMake 3.28+, Ninja, ICU, libxml2, libcurl, and Xerces-C 3.2+ for XML Schema
support. libxslt is optional (XQueryX).
Stock Xerces-C (3.3.0 and earlier) rejects some valid schemas that restrict
a union type; the Linux CI images build Xerces-C from a patched fork, see
`KNOWN_ISSUES.txt`.

CI builds and tests Linux x86_64 (GCC and Clang), Linux arm64, and macOS
arm64. macOS x86_64 should build but is not tested in CI. Windows is not
supported.

Debian / Ubuntu:

```bash
sudo apt-get install build-essential cmake ninja-build pkg-config \
  libicu-dev libxml2-dev libxslt1-dev libcurl4-openssl-dev zlib1g-dev \
  libxerces-c-dev uuid-dev
```

macOS (Homebrew):

```bash
brew install icu4c libxml2 xerces-c ninja
export CMAKE_PREFIX_PATH="$(brew --prefix icu4c):$(brew --prefix libxml2):$(brew --prefix xerces-c)"
```

Configure, build and install with the presets in `CMakePresets.json`:

```bash
cmake --preset full-release -DCMAKE_INSTALL_PREFIX=/usr/local/zorba
cmake --build --preset full-release
cmake --install build/full-release
```

Or run configure, build and test in one step with a workflow preset:

```bash
cmake --workflow --preset full-release           # needs the HTTP test server
cmake --workflow --preset full-release-offline   # skips network tests
```

Presets: `core-release` (no XML Schema), `full-release`, `full-debug`. The
installation prefix is fixed at configure time.

By default `xs:integer` is a 64-bit `long long`: arithmetic, `fn:sum` and the
like raise `err:FOAR0002` when a result does not fit, casts of out-of-range
values to `xs:integer` raise `err:FOCA0003`, and JSON integers that do not fit
are parsed as `xs:decimal`. The `bigint` preset
(`ZORBA_WITH_BIG_INTEGER=ON`) makes `xs:integer` arbitrary precision instead,
using the bundled MAPM library that already backs `xs:decimal`. MAPM keeps
static working buffers and is not thread-safe, so do not run queries
concurrently in one process with that build (see `KNOWN_ISSUES.txt`).

`docker/builder` (Ubuntu 24.04) and `docker/builder-latest` (Arch Linux,
latest toolchain and libraries) provide ready-made build environments.

## Testing

```bash
ctest --preset full-release-offline -j8   # everything except network tests
```

The http-client tests need the HTTP test server in `test/http-test-data`
(see its README). With Docker:

```bash
docker build -t zorba-http-test -f docker/http-test-server/Dockerfile .
docker run -d -p 8080:8080 zorba-http-test
ctest --preset full-release -j8
```

`docker/test-with-http-server.sh` runs the whole suite in a builder container
against that server.

## Documentation

Doxygen and XQDoc sources are in `doc/`.

## License

Apache License 2.0; see `LICENSE.txt` and `NOTICE.txt`.
