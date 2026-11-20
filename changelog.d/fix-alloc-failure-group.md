### Fixed

- A failed allocation while a cached function stores a result no longer leaves
  a function cache entry without its value, which the cache later released as
  if it owned a reference to the key.
- Fix a memory leak when `group by` fails, for example when memory runs out,
  while it starts a new group: the sequences of the group's non-grouping
  variables are now freed.
- Fix a memory leak when the optimization of a function body, done on its
  first dynamic call, fails, for example when memory runs out, while the index
  join rule starts.
