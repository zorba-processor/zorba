### Changed

- An object built from a known number of pairs (by `parse-json`, an object
  constructor or a function that returns an object) takes one allocation
  instead of two: its pairs are stored in the object's own allocation until an
  update makes it outgrow that room.
