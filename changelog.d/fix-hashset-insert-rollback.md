### Fixed

- An internal hash set insert whose key copy fails, for example when memory
  runs out, no longer leaves an entry with an empty key counted in the set.
