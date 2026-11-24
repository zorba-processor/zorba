### Changed

- In builds with `ZORBA_FOR_ONE_THREAD_ONLY=OFF`, a function item created by
  a query that declares or assigns variables (scripting) can only be called
  on the thread that created it; a call on another thread raises
  `zerr:ZXQD0007`. Its calls write variables that calls on several threads
  would share (the local and captured variables of its body, the prolog
  variables of its query), which was undefined behavior before. On its own
  thread it still works after its query is closed, and the variables it
  writes are written under the lock of the cycle collector, so that a
  collection started by another thread does not walk them meanwhile.
