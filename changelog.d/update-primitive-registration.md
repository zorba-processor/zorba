### Fixed

- An update (insert, delete, replace, rename, revalidation, a collection,
  index, document or hashmap update, or a JSON update) that ran out of memory
  while it was added to the pending update list no longer keeps its target in
  memory until the process ends. With a target typed by `xsi:type`, the store
  reported `ZSTR0065` at shutdown.
