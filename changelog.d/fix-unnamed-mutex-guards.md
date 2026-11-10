### Fixed

- In builds configured with `ZORBA_FOR_ONE_THREAD_ONLY=OFF`, hold the Xerces
  grammar pool lock for the whole schema operation (schema import, type and
  declaration lookups, user-defined atomic parsing, schema serialization);
  each lock was released in the statement that took it. `testdriver_mt` had
  the same defect on its per-query lock.
