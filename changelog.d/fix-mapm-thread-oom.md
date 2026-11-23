### Fixed

- Fix the first decimal arithmetic of a thread ending the process ("Fatal
  glibc error: failed to register TLS destructor: out of memory") when memory
  ran out at that point: the per-thread state of the decimal library was a
  `thread_local` with a destructor, whose registration glibc cannot report
  as an error. The state is now freed at thread exit through a pthread key,
  and running out of memory while setting it up is reported as any other
  allocation failure.
