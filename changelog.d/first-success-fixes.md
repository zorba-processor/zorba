### Fixed

- Fix `--module-path` with a relative path such as `.` under `--secure` or any restricted read policy: the module was refused with XQST0059 and "requires capability RemoteImport" because the relative path was not resolved against the current directory. A path with `..` that leaves the granted directory is still refused.
- Fix `--profile json` (and the XML and DOT profile outputs) printing garbage counters, such as `"prof-next-calls": 4294967295` with `"prof-calls": 0`, and `nan` CPU times, for iterators that were never opened, such as the body of a cached function call. The profile counters now start at zero and the JSON printer never writes `nan` or `inf`.
- Fix an installation made with `cmake --install --prefix <dir>` or moved after installing: the binaries failed with `libzorba_simplestore.so: cannot open shared object file`. The RPATH is now relative to the binary, so the binaries start from any prefix. The module and library directories are still fixed at configure time.
- Correct the description that `zorba-mcp` sends to MCP clients: it lists HTML among the supported formats.

### Changed

- Documentation: the examples of the README, the container README and the web site show runnable commands (`-r` for `-q '1 + 1'`, a query file created in the snippet, the C++ compile command), the thread-safety text states only the current behavior, and the site examples are checked against the `edge` image.
