### Fixed

- Inserting an element no longer leaves the type of its hidden base-uri
  attribute in the target tree after that attribute is freed: a node later
  allocated at the same address and added to the tree's types failed an
  assertion, or in a release build kept the stale entry.
- Adding a member to a JSON object no longer leaks the collection's update list
  or the insert primitive when an allocation fails while they are registered.
