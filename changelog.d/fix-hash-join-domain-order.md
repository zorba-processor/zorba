### Fixed

- Fix a general comparison join (`=`) over an inner domain of nodes that are
  not in document order, for example `fn:reverse($d/b)`: at the default and
  higher optimization levels the join returned the matching nodes in document
  order instead of the order of the domain. Such a join is now evaluated
  without an index.
