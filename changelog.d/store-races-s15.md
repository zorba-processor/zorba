### Fixed

- In builds configured with `-DZORBA_FOR_ONE_THREAD_ONLY=OFF` (unsupported),
  threads asking for the first time for the `fn:generate-id` or the
  `ref:reference` of the same node could get different ids and corrupt the
  store's reference maps. Assigning a node's reference is now atomic, so all
  threads get the same id. Default builds are unaffected.
- In builds configured with `-DZORBA_FOR_ONE_THREAD_ONLY=OFF` (unsupported),
  after a thread's upgrade of the store lock from read to write had to wait
  for other readers, the lock kept the upgrade request: a later upgrade that
  had to wait failed with `ZXQP0030` (deadlock), and debug builds stopped on
  an assertion when the lock was released.
