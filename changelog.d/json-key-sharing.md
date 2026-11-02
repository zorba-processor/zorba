### Changed

- Reduce the memory of parsed JSON by 56% and of `csv:parse` results by 37% on
  the `bench/store` inputs, and their parse time by 12%: the objects of one
  `jn:parse-json` share their key items, and objects of at most 16 pairs have
  no key hash map.

### Fixed

- Fix a memory leak in `jn:parse-json` when an object has a duplicate key
  (JNDY0003).
