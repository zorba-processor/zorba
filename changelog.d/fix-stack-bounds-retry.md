### Fixed

- When reading the bounds of a thread's stack fails for lack of memory, the
  stack limit reads them again on its next call instead of leaving the thread
  without a stack check for its life; other failures are still remembered.
