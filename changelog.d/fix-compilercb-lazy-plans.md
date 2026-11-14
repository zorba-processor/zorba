### Fixed

- A sliding window in the body of a function loaded with
  `StaticContext::loadProlog()`, or of a function item made by another query,
  read its input as it went when the query that runs it can run a side effect:
  the side effect could change the input between two windows. Such a window now
  reads its input before the first window, as in a query of its own that can run
  a side effect.
- A tumbling window read its input as it went even when the query that runs it
  can run a side effect. It now reads its input before the first window then,
  like a sliding window.
- Calling `next()`, `open()` or `close()` on the result iterator of a closed
  query crashed; it raises ZAPI0042.
