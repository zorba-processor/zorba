### Fixed

- Fix wrong results of `ref:reference` and `ref:has-reference` called directly at
  optimization levels O1 and O2: the optimizer treated both as pure and could
  hoist or merge a call of `ref:has-reference` across the `ref:reference` call
  that changes its result. Both functions are now declared nondeterministic.
