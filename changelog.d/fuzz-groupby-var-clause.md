### Fixed

- Fix an invalid read in the optimizer after it rewrites a `group by`
  clause that sees at most one tuple into `let` clauses: the domain of a
  grouping or non-grouping variable was then read as if its clause were
  still a `group by` clause.
