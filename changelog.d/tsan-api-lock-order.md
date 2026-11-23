### Fixed

- In builds configured with `-DZORBA_FOR_ONE_THREAD_ONLY=OFF` (unsupported),
  `Zorba::getInstance` on a running engine took the engine's users mutex. An
  external function that calls it (the modules shipped with Zorba do, for
  their item factory) holds the store lock, and the first `getInstance` takes
  the store lock while holding that mutex: a lock-order inversion. It now
  registers the new user without the mutex.
