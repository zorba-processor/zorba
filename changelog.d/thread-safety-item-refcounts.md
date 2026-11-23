### Fixed

- Fix races on item reference counts in the engine built with
  `-DZORBA_FOR_ONE_THREAD_ONLY=OFF`: a node moved to another tree while
  another thread released a reference to it could leave a tree lock held or
  release a lock not held, and a QName released while a lookup found it could
  be removed from the pool twice. Item counts are now atomic in that build
  instead of a lock per item, and a node's count changes under its tree's
  lock after checking that the node is still in that tree. The booleans,
  small integers, `null` and the store's type names are no longer counted at
  all, in either build.
