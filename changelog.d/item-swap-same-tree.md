### Fixed

- Fix, in builds with `ZORBA_FOR_ONE_THREAD_ONLY=OFF`, a hang when the store
  swaps two nodes of the same tree: it locked the tree twice with a lock that
  is not recursive. It now locks the tree once.
