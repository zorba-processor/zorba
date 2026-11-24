### Fixed

- `math:quartile` sorts its input for the first and third quartiles: on
  unsorted input it no longer interpolates between the values at the
  unsorted positions. On a single value it returns that value for every
  quartile instead of raising `XPTY0004`.
