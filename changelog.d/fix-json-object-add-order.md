### Fixed

- An insert into a JSON object that fails when memory runs out leaves the
  object unchanged; before, the object could keep the member, or a later
  lookup by name could return the value of another member.
