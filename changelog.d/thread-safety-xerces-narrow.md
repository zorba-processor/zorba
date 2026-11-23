### Changed

- In builds with `ZORBA_FOR_ONE_THREAD_ONLY=OFF`, validations and casts to
  user-defined types run in parallel on several threads, also against one
  schema. Before, every schema operation took one process-wide mutex, so
  validation did not scale with threads. Schema imports and the creation of a
  schema still run alone: they wait for the validations in progress, and new
  validations wait for them.
