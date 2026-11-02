### Changed

- Reword the file and I/O error messages (FODC0001, FODC0002, FODC0004, FODC0007, FOUT1170, FOUT1190, ZOSE0002,
  ZOSE0003, URI_UNRESOLVED_OR_NOSTREAM) to say what failed in fewer words. FOUT1190 names the unsupported encoding,
  and FOUT1170 says when the URI has a fragment identifier or is not a valid URI. Error codes do not change.

### Fixed

- `fn:collection()` without an argument and no default collection reported "default collation" instead of
  "default collection".
- The functions of the `http://zorba.io/modules/store/documents` module (`put`, `remove`, `document`,
  `is-available-document`) reported an empty URI in FODC0004 when the URI could not be resolved.
