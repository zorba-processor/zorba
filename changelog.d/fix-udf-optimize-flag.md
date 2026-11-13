### Fixed

- When the optimization of a function body fails, for example when memory runs
  out during its first dynamic call, the next run of the same compiled query
  optimizes the body again instead of running it as the failed optimization
  left it.
