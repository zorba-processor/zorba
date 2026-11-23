### Fixed

- Fix, in builds with `ZORBA_FOR_ONE_THREAD_ONLY=OFF`, races and crashes when
  the function items of a query are called, copied and released on other
  threads, also while or after the query is closed: closing the query handed
  its state over to its function items without publishing it, the collection
  of the cycles through the prolog variables of a closed query walked a graph
  that other threads were changing or freeing, two threads could walk it at
  once, and releasing a reference to such a function item read the item after
  another thread could have freed it. The collections, and the releases of
  references to function items of a closed query, now run under one lock (the
  store lock in read mode, then a process-wide mutex), and a walk that finds
  garbage releases nothing if a reference count changed during the walk.
