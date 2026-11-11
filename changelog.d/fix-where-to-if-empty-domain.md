### Fixed

- Fix a where clause that does not use the variables of its FLWOR raising an
  error at O1 and O2 when no tuple reaches it: its condition was evaluated
  before the FLWOR, and is now moved there only if it cannot raise an error or
  the clauses before it produce at least one tuple.
