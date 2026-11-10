### Fixed

- Fix `ddl:delete` raising ZDDY0015 for a collection after a sequential
  function that bound a node of the collection to a local variable (a
  `variable` declaration, a `let` or a `for`) had returned: the local
  variables kept their values after the function returned. A sequential
  function is no longer inlined into its caller, and its call releases its
  local variables when it completes.
