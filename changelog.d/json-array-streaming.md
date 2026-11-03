### Changed

- `parse-json($text)[]` and `members(parse-json($text))` read the members of a
  top-level array one at a time, without building the array. Peak memory follows
  the largest member, plus the text unless it is read with `file:read-text` in a
  query with no side effects. The array is parsed twice: first in full, so that
  errors stay the same and come before any member, then to return the members.
