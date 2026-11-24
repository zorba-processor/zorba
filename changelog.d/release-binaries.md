### Added

- Each release ships binary tarballs for Linux x86_64, Linux arm64 and macOS
  arm64 with `zorba`, `zorba-mcp`, the headers and the Zorba library, the
  libraries they need bundled in `lib/` and found relative to the binaries,
  so a tarball runs wherever it is unpacked.

### Fixed

- A Zorba installation that is moved or unpacked elsewhere than the prefix it
  was configured with finds its module and schema files and module libraries:
  Zorba looks for them relative to the directory of its library first, where
  it used to look only under the configured prefix and the build tree.
