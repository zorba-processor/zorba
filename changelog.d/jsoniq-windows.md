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
- Count the windows of a FLWOR whose return clause is a constant, as in
  `count(for sliding window ... return 1)`. It gave 1 at any number of windows.
- Count a FLWOR with `allowing empty` over an empty sequence.
- Return nothing for `$x[0]` and `$x[-1]` on a variable. They returned the
  whole value of the variable.
- Raise XPST0003 for a window clause whose conditions are not a start condition
  followed by an end condition, or for a sliding window without an end condition.
