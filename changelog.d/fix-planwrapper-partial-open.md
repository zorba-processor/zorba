### Fixed

- Fix a memory leak when opening a query plan fails partway, for example when
  memory runs out while a FLWOR expression with `group by` or `order by` sets
  up its state: the iterator states created before the failure are now
  destroyed.
