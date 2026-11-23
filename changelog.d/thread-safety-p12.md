### Fixed

- Fix, in builds with `ZORBA_FOR_ONE_THREAD_ONLY=OFF`, races when queries are
  compiled on several threads against one static context: the first
  compilations set the context's type manager, the first schema import,
  validation or cast to a user-defined type created the schema of that type
  manager, and the optimizer took the ids of temporary indexes from one
  unsynchronized counter. A static context created through the API now has
  its type manager from the start, and its schema is created once.

### Changed

- In debug builds with `ZORBA_FOR_ONE_THREAD_ONLY=OFF`, changing a static
  context (its setters, `loadProlog()`) after a query was compiled against it
  raises `ZXQP0002`, since queries on other threads may be reading it: a
  context is set up completely before it is shared.
