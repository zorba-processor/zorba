### Fixed

- At O1 and O2, a `for` clause whose domain has at most one item is no longer
  folded into the rest of the FLWOR when a later clause can raise an error.
  When the domain was empty, the later clauses were evaluated after the fold
  and could raise an error that O0, which has no tuples to evaluate them for,
  does not raise.
