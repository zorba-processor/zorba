### Fixed

- Fix a document read with the `dtd` feature when libxml2 runs out of memory
  while it parses: the load failed with `err:FODC0002` ("XML tree creation
  failed" or "malformed XML document"), an error a catch clause could catch.
  It now fails as running out of memory does elsewhere in the engine. With
  libxml2 before 2.13, which does not report every allocation of its own that
  fails, some of these failures still give `err:FODC0002`.
