### Fixed

- In builds configured with `-DZORBA_FOR_ONE_THREAD_ONLY=OFF` (unsupported),
  `ref:dereference` of a node whose last reference another thread was
  releasing could return the node while its tree was being destroyed, a
  use-after-free. It now returns the empty sequence for such a node. Default
  builds are unaffected.
