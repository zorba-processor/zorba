### Fixed

- Fix a JSON value failing to enter a collection when memory ran out staying
  partly attached to it, with its collection data leaked, and a JSON value that
  failed to enter an object or array of a collection keeping the collection's
  data, read after the collection was gone. Giving a JSON tree its collection
  data, or taking it back, now allocates nothing, so the undo of an update of
  JSON in a collection no longer fails with "Unexpected error during pul undo".
