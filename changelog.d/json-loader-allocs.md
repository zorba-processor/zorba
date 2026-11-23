### Changed

- `parse-json` and `jn:parse-json` allocate less per JSON object and array:
  the loader reuses the key and value vectors of each nesting level instead
  of allocating and growing new ones for every object and array.
