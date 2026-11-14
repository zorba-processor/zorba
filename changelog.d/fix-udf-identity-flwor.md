### Changed

- When an index probe replaces a `for` clause and leaves a FLWOR that only
  returns the probed items, the optimizer now drops that FLWOR, in function
  bodies and in the main module alike.
