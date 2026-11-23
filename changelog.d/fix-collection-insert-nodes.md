### Fixed

- Fix nodes inserted before or after a node of a collection
  (`dml:insert-before`, `dml:insert-after`) staying attached to the collection
  when the insert ran out of memory: they were not in it, and a later insert of
  the same nodes failed with `ZSTR0010`. The collection now grows first, and a
  node it cannot attach detaches the ones attached before it.
