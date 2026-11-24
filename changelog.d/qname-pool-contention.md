### Changed

- In builds with `ZORBA_FOR_ONE_THREAD_ONLY=OFF`, lookups of names that are
  already in the store's QName pool run in parallel on several threads. Before,
  every lookup took the pool's mutex and the namespace pool's mutex. A lookup
  of an existing name no longer touches the namespace pool in any build.
