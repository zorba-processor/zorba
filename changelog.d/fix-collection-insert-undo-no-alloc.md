### Fixed

- Fix a failed update that inserted nodes into a collection (`dml:insert`,
  `dml:insert-last`, `ddl:create` with nodes) ending the process ("Unexpected
  error during pul undo") in builds with `ZORBA_WITH_BIG_INTEGER`, when memory
  ran out: the undo made big integers for the positions of the nodes it
  removed, and that allocation failed too. It now removes them without
  allocating.
