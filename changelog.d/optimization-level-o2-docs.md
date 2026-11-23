### Fixed

- The documentation of optimization level 2 (`ZORBA_OPT_LEVEL_O2` in the C++
  API, `zorba --optimization-level 2`) says what it does: the same as level 1.
  It said that level 2 added optimizations, such as not generating an iterator
  for inlined functions, and did not generate call stacks for errors; the
  compiler does neither and treats level 2 exactly as level 1.
