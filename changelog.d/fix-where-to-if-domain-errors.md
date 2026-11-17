### Fixed

- At O1 and O2, a `where` clause whose condition does not use the variables of
  its FLWOR is moved in front of the FLWOR (`if E then flwor else ()`) only
  when the clauses before it cannot raise an error. Before, a false condition
  skipped those clauses, and the error that O0 raises while evaluating them,
  such as a `for` domain that divides by zero, was lost.
