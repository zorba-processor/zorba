### Fixed

- Fix, in builds with `ZORBA_FOR_ONE_THREAD_ONLY=OFF`, races when queries are
  compiled on several threads against one static context: the first
  compilations set the context's type manager, a query's schema import
  created or extended the schema of the type manager the other queries used,
  the first validation or cast to a user-defined type created that schema,
  and the optimizer took the ids of temporary indexes from one unsynchronized
  counter. A static context created through the API now has its type manager
  from the start.

### Changed

- A schema that a query imports is no longer visible to the other queries
  compiled against the same static context, unless the prolog loaded into
  that context (`loadProlog()`) imported a schema: then the query's imports
  are still added to that schema.
- In debug builds with `ZORBA_FOR_ONE_THREAD_ONLY=OFF`, changing a static
  context (its setters, `loadProlog()`) after a query was compiled against it
  raises `ZXQP0002`, since queries on other threads may be reading it: a
  context is set up completely before it is shared.
