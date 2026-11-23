### Fixed

- Fix a failed update that creates or deletes collections ending the process
  ("Unexpected error during pul undo") when memory ran out: putting back a
  collection that `ddl:delete` took out of the store could grow the store's map
  of collections, which the deletes before had shrunk. `ddl:create` and
  `ddl:delete` now reserve room in that map, so their undo allocates nothing.
