### Added

- `zorba-mcp` scans the directory in the background: `list_documents` answers
  at once with what is known and a status, takes `path`, `limit` and `refresh`,
  and gives a short preview of a file (CSV columns, JSON key paths, XML element
  paths, HTML tables) with samples, read from at most 1 MiB of the file.
- Add the `--no-scan` option to `zorba-mcp` to keep the previous behavior, and
  `--scan-cache-mb` to limit the memory of the scan's file index and previews,
  with least-recently-used replacement of previews on request.

### Changed

- `list_documents` in `zorba-mcp` starts with a status line, lists 100 files by
  default with the total, and no longer gives `modified` (use `--no-scan` or
  `dir:list`). Its listing can miss files while the status says `running`;
  query results do not depend on it.
