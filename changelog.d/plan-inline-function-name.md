### Changed

- The iterator tree printed by `--iterator-tree` (and the profile output) names
  an inline function by the file name of its query and its location, for
  example `inline-function(q.xq:1.12-1.35)`, instead of the full path of the
  query file, so that the printed plan does not depend on where the query is.
  Error messages keep the full path.
