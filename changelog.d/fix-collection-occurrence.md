### Breaking changes

- The occurrence indicator of a static collection's declared type is checked
  when the collection is read: `dml:collection` raises `err:XPTY0004` when
  the number of items in the collection does not match it (for example two
  elements in a collection declared `as element(*)`), with a message naming
  the collection, its size and its declared type, at every optimization
  level. Before, the indicator was not checked, and O1/O2 rewrote queries on
  it (a `count` folded to 1, a `let` turned into a `for`), so results differed
  from O0. Inserting is not checked. Declarations that relied on the indicator
  being ignored need `*`.

### Fixed

- XQDDF: the queue rule of `delete-nodes`, `iddl:delete` raising `ZDDY0023`,
  and the errors of `cdml:delete-first` and `cdml:delete-last` (`ZDDY0004`,
  `ZDDY0007`, `ZDDY0008`) now match the implementation.
