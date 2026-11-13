### Fixed

- A sliding window in the body of a function loaded with
  `StaticContext::loadProlog()`, or of a function item made by another query,
  read its input as it went when the query that runs it can run a side effect:
  the side effect could change the input between two windows. Such a window now
  reads its input before the first window, as in a query of its own that can run
  a side effect.
