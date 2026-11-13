### Fixed

- An internal hash map insert whose key copy fails, for example when memory
  runs out, no longer leaves an entry with an empty key counted in the map.
