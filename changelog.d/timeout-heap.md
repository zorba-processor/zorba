### Fixed

- Fix a crash when a query stopped during an `order by` sort, by the time limit
  or by a type error in the comparison of two keys: the sort keys were released
  incorrectly afterwards.
