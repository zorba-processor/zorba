### Fixed

- Fix JSONiq array updates with a delete and a replace at the same position of
  one array in one pending update list: the member is now deleted and the
  replace is omitted, as the JSONiq update facility's merging rules require;
  before, the replace was applied and the delete dropped. Two replaces at that
  position still raise `jerr:JNUP0009`.
