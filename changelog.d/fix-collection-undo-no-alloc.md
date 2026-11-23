### Fixed

- Fix a failed update of a collection ending the process ("Unexpected error
  during pul undo" or "during pul apply") when memory ran out: the undo of
  `dml:insert-first`, `dml:insert-before`, `dml:insert-after` and
  `dml:delete`, and the final pass that renumbers the trees of a collection,
  allocated, for big integers in builds with `ZORBA_WITH_BIG_INTEGER` and for
  the collection data of each tree put back by the undo of a delete. They now
  allocate nothing.
- Fix the undo of a `dml:delete` of several trees of a collection putting them
  back in a different order, and the undo of `dml:insert-last` leaving the
  inserted trees in the collection.
- Fix a tree that failed to enter a collection when memory ran out staying in
  the collection without being attached to it.
