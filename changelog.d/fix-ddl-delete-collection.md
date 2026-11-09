### Fixed

- Fix `ddl:delete` raising ZDDY0015 for a collection that had an automatic
  index while nodes were inserted, after the index was deleted: index
  maintenance kept the last indexed node referenced, which `ddl:delete` took
  for a variable that references a node of the collection.
