### Fixed

- Fix schema validation (`validate`, and the option checks of functions such as
  `x:canonicalize`) when an allocation fails: the validator kept up to about
  170 KB per failed call, and the failure was reported as the internal error
  `ZXQP0003` instead of running out of memory.
