### Fixed

- Fix a failed update leaving an element or attribute without its schema type:
  when memory ran out while an update took the type away, or before an insert
  attached its first node, the undo did not give the type back.
