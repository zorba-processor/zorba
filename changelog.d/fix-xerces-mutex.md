### Fixed

- In builds configured with `-DZORBA_FOR_ONE_THREAD_ONLY=OFF` (unsupported),
  the Xerces lock around schema import, validation and casts to user-defined
  types was released in the statement that took it, so it protected nothing.
  It is now held for the whole operation and is recursive, since validation
  calls schema functions that take it again. Default builds compile the lock
  out and are unaffected.
