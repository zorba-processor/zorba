### Changed

- At `-O1` and `-O2`, a global variable whose only reference is the domain of
  the first `for` or window clause of the query body (after `let` clauses only)
  is computed in that domain, streamed, instead of being materialized before
  the body runs: `declare variable $events := parse-json(...)` followed by
  `for $e in $events ...` now uses the memory of the `let` form. Results and
  errors are those of `-O0`, except that when more than one error can be
  raised, an error of the variable can give way to one of a global declared
  after it or of the query body. Through the C++ and C APIs, reading such a
  variable after execution (`DynamicContext::getVariable`, XQC
  `get_variable`) raises `XQDY0054` instead of returning its value. A caller
  that stops reading the results before their end can skip an error of the
  variable that `-O0` raises before the first item.
