### Fixed

- Fix a leak of the state of a closed query kept alive through a cycle (a
  function item stored in a prolog variable) when the only reference from
  outside the cycle is a JSON object or array of the query's state, e.g. a
  prolog variable value that holds the function item and that the
  application kept from the query result. Dropping that reference after the
  query was closed collected nothing. A collection that finds such a JSON
  item now marks it, and dropping a reference to a marked item collects the
  cycles of the closed queries.
