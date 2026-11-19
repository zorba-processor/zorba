### Fixed

- `insert nodes ... into`, `as first into` and `as last into` with both
  attributes and other nodes crashed the process: the attributes' update
  took the target, and the other nodes' update got none.
