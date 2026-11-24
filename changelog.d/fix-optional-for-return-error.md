### Fixed

- A `for` clause over a domain of at most one item is folded at O1 and O2 only
  if the return clause cannot raise an error or reads the variable first, so
  an empty domain no longer evaluates a return clause that raises where O0
  gives the empty sequence.
