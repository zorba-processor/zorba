### Fixed

- When memory runs out while a member is added to a JSON object, a later
  lookup by name no longer returns the value of another member, and the object
  no longer keeps a member whose insert failed while it built its index of
  names.
