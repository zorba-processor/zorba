### Added

- Directory module (`http://zorba.io/modules/directory`): query a directory tree of XML, JSON and CSV files. `dir:list($root)` lists the files (relative path, format, size, modification time); `dir:open($root, $path)` and `dir:documents($root, $formats)` return document records `{ "path", "format", "content" }` (a document node for XML, the parsed item for JSON, an array of row objects for CSV) for joins across files and formats. A file that cannot be parsed yields `{ "path", "format", "error" : { "code", "namespace", "message" } }` instead of stopping the query. HTML files are listed but not parsed yet. `$dir:root` holds a default root bound by the embedder or the command line.
- `zorba --directory <dir>`: runs the query with the directory module's `$dir:root` bound to `<dir>` and writes the result as JSON, one item per line.

### Security

- `zorba --directory <dir>` runs the query under the secure policy with only `file-read` (restricted to `<dir>`) and `native-modules` allowed; options that would widen it (`--allow`, `--read-path`, `--write-path`, `--network-prefix`, `--private-addresses allow`) are rejected. The directory module rejects paths with `..` segments, absolute paths and paths through symbolic links (`dir:INVALID_PATH`), and its listing does not follow symbolic links.

### Changed

- The JSON serialization method writes the `item-separator` serialization parameter, when set, between top-level items (it was ignored).
