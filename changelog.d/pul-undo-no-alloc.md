### Fixed

- Undoing applied updates (inserts and replacements of nodes, and the merge
  of adjacent text nodes) no longer aborts the process when no memory is
  left: what the undo needs is kept while the update is applied, and the
  undo allocates nothing.
