### Fixed

- Fix data races on the reference counts of compiler and runtime objects
  (functions, type managers, user-defined types, plan iterators, store
  iterators) in the engine built with `-DZORBA_FOR_ONE_THREAD_ONLY=OFF`: their
  counts are now atomic in that build. Builtin functions and the root static
  context, which every query shares, are no longer counted at all, in either
  build.
