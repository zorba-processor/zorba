### Fixed

- A sequential function that raised an error after binding a node of a
  collection to a local variable kept that node referenced when the caller
  caught the error: a later `ddl:delete` of the collection raised ZDDY0015.
  The body of a call that raises is now released as if the call had completed.
