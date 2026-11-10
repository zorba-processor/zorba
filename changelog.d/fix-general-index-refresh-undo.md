### Fixed

- Fix the undo of a general index refresh when an update fails after removing
  a node with more than one key from the index: the undo counted the node's
  second key against the wrong delta, so it restored too few keys and could
  crash.
