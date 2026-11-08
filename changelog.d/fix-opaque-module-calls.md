### Fixed

- Fix wrong results of `jsd:validate`, `jsd:annotate` and
  `icdml:check-integrity-constraint` in a loop at optimization levels O1 and
  O2 when the code they run has a side effect: a constraint of the JSound
  schema (a query from a string) or the body of the integrity constraint. The
  optimizer treated the call as pure and could run it once for all
  iterations. These calls are now treated like a call of `zq:evaluate`, whose
  code the compiler cannot see.
