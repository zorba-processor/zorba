### Fixed

- Build the index of a hash join at its first probe: a join under an empty
  outer domain, or one whose outer tuples never reach it, no longer evaluates
  its inner keys, which the query without the index does not evaluate either
  and which may not end. A join whose index is built inside a block of a
  sequential expression still builds it in advance.
