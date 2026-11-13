### Fixed

- A failed allocation while a cached function stores a result no longer leaves
  a function cache entry without its value, which the cache later released as
  if it owned a reference to the key.
