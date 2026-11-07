### Fixed

- Fix an invalid read in the optimizer when a FLWOR expression whose
  `group by` clause sees at most one tuple is rewritten: the grouping and
  non-grouping variables were moved into `let` clauses but kept their
  group-by kind.
