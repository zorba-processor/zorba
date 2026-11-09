### Fixed

- Fix the index maintenance of an update that truncates a collection while it
  also modifies another collection that has an automatic index: the index of
  the other collection was cleared and the index of the truncated collection
  kept its entries.
