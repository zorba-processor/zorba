### Fixed

- Fix JSONiq array updates on one array in one pending update list: an insert
  and a replace (or delete) at the same position now both refer to the
  original member at that position, as the JSONiq update facility requires;
  before, the replace could hit the newly inserted member. The order in which
  array updates are applied is also now a well-defined ordering instead of an
  invalid sort comparator.
