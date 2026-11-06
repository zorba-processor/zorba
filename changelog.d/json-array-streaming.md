### Added

- Add the option `zorba-streaming-members` of `parse-json`. With it,
  `parse-json($text, { "zorba-streaming-members" : true })[]` reads the members of
  a top-level array one at a time, without building the array, so that peak memory
  follows the largest member, plus the text unless it is read directly with
  `file:read-text` in a query with no sequential expression. Each top-level array
  is parsed twice: first to check it, so that errors and their order stay the same,
  then to return its members. Without the option nothing changes.
