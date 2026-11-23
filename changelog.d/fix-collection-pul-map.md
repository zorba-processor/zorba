### Fixed

- Fix a failed update over two or more collections leaking the part of an
  update list for one collection: when memory ran out while two update lists
  were merged, that part was in neither list. The part for a collection is now
  in the list and in its index by collection, or in neither.
