### Fixed

- Fix a memory leak in `jn:parse-json` (and JSONiq `parse-json`) when memory
  runs out while an object member is added: the member's key was not freed.
