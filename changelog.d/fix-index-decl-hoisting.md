### Fixed

- Restore hoisting and index joins in the expressions of index declarations
  (`declare index`: the domain and every key). They were optimized as if the
  query contained a call that can have a side effect, and no expression was moved
  out of a loop or joined through a temporary index in any query. They are
  optimized with hoisting now, unless one of them makes a call through a function
  item or an eval. The rule of the query that imports the declaration does not
  change.
