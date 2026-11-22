### Fixed

- Fix the last release of a QName item in builds with
  `ZORBA_FOR_ONE_THREAD_ONLY=OFF` unlocking the item's reference-count lock a
  second time, after the count had already released it; with spin locks that
  can unlock a lock another thread holds.
