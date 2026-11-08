### Fixed

- Fix `file:write-text`, `file:write-text-lines`, `file:write-binary` and the
  `file:append-*` functions returning success with a truncated file when the
  content is a streamable item and its stream cannot be read, or when the file
  cannot be written (disk full, I/O error). Both now raise `file:FOFL9999`
  naming the path. An empty streamable content still writes an empty file
  (and, for the `-lines` functions, the line break).
