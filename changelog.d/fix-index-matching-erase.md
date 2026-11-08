### Fixed

- Fix the automatic index rewrite reading a predicate after removing it from
  its working list: the rule took the position of the last matched `where`
  clause from the next predicate, so an index that applies was not used when
  a later `where` clause followed a `count` or `group by` clause. Query
  results do not change.
