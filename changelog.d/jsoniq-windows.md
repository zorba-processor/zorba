### Added

- Add JSONiq window examples to the site, with moving averages, sessions and
  change detection over irregular records.

### Fixed

- Bind a sliding window to its own start item when an earlier window with
  `only end` never closed. It was bound to the start item of that earlier window.
- Return nothing for a position past the end of a window variable, as in
  `$w[2]` on a window of one item. It returned the next item of the input.
- Accept `fn:tail`, `subsequence($w, 2)` and `$w[position() gt 1]` on a tumbling
  window. They failed with an internal error.
- Raise XPST0003 for a window clause whose conditions are not a start condition
  followed by an end condition, or for a sliding window without an end condition.
