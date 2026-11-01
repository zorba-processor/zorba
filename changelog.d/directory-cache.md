### Added

- `zorba-mcp --cache-mb <MiB>` (default 256, 0: off): the documents that the directory module parses are kept across tool calls, up to that much estimated memory (file size times 20 for XML, 22 for JSON, 32 for CSV, as measured), least recently used first; entries used by the current call are not evicted, so a call that reads more than the budget keeps the documents it cached first. A document is served from the cache only while its file has the same canonical path, device, inode, size, modification time and change time; entries of modified or deleted files are dropped before each call. Only files under `--root` are cached, the security policy is unchanged, and updating or sequential queries do not use the cache.
- Module `http://zorba.io/modules/directory-cache`: the hook through which the directory module looks up and offers parsed documents. The installed implementation caches nothing; an embedding application can resolve the module to its own external functions.

### Changed

- `dir:documents` sorts the paths before parsing, so a query that consumes the documents one at a time keeps one parsed document in memory instead of all of them.
