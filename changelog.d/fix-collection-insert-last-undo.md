### Fixed

- Fix a failed update that inserted nodes last into a collection
  (`dml:insert-last`) leaving those nodes in the collection: its undo did not
  count the nodes it had inserted, and removed none. It now removes them, the
  last one first.
