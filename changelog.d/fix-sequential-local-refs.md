### Fixed

- Fix `ddl:delete` raising ZDDY0015 for a collection after a sequential
  function that bound a node of the collection to a local variable (a
  `variable` declaration, a `let` or a `for`) had returned: the call kept the
  values of its local variables after it returned.
