### Fixed

- In builds with `ZORBA_FOR_ONE_THREAD_ONLY=OFF`, the function items of one
  query can be called on several threads at once also when their calls
  assign variables in an eval query, exit, apply updates or call sequential
  external functions. Each such call advances the snapshot of the query that
  created the item (which function result caches compare), and concurrent
  calls advanced it in a data race; the snapshot is now advanced atomically.
  An eval query assigns its own copy of the outer query's variables, so its
  assignments never change a variable of the outer query.
