### Fixed

- Fix two inserts into the same object, merged into one update list, keeping
  part of the second when it repeats a name of the first (JNUP0005): the
  insert in the list kept the names before the repeated one and empty places
  for the rest. It is now checked for repeated names first, and stays as it was.
