### Security

- `zorba` command line: new options restrict what a query may do. `--secure` starts from a policy that allows nothing; `--allow` and `--deny` take a comma-separated list of capabilities (`external-entities`, `file-read`, `file-write`, `network`, `remote-import`, `environment`, `native-modules`, `all`) and are applied in command-line order; `--read-path`, `--write-path` and `--network-prefix` (repeatable) limit file and network access to directories and URL prefixes; `--private-addresses allow|deny` controls access to loopback, private and link-local addresses. Without these options `zorba` still runs queries unrestricted.
- CI runs `scripts/check-os-access.sh`, which fails when code in `src/` or `modules/` opens files, loads libraries, reads the environment, connects to the network or parses XML without an entry in the reviewed allow-list `scripts/check-os-access.allow`.

### Added

- `zorba --max-call-depth`, an alias for `--max-udf-call-depth`.

### Changed

- `zorba --disable-http-resolution` is now the same as `--deny remote-import`.
