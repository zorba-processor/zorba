### Changed

- Parse-time projection of `parse-json` at `-O1` and `-O2` follows `let`
  clauses: a `let` bound to the values (an alias of the `for` variable, or a
  FLWOR or predicate that returns them) is analyzed as the `for` variable is,
  and a `let` or `for` bound to a lookup path (`let $http := $e.http`), also
  collected by a `group by`, builds only the keys looked up in it
  (`$http.status`) instead of the whole value at that path. A use of such a
  variable other than a lookup of a literal key builds that value whole.
