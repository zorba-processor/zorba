### Fixed

- Fix a typed element (an element constructed with `declare construction
  preserve`, a copy that keeps types, a validated element) leaking its tree's
  map of types when memory ran out while the element was made: a tree freed
  before it was complete did not free that map.
