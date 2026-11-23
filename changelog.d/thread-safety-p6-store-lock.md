### Fixed

- Fix, in builds with `ZORBA_FOR_ONE_THREAD_ONLY=OFF`, sequential queries
  running under the store's read lock: their updates were visible to queries
  on other threads while the sequential query was still running. A sequential
  query now holds the store's write lock for its whole run, like an updating
  query.
- Fix, in builds with `ZORBA_FOR_ONE_THREAD_ONLY=OFF`, a deadlock between a
  compilation that imports a schema whose `xs:include` or `xs:import` is
  fetched by a query (the HTTP resolver, or an application's URL resolver
  that runs one) and a query on another thread that validates. Compilation
  (`compile()`, `loadProlog()`) now holds the store's read lock.
