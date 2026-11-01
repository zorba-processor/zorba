### Changed

- Unchecked results of pthread, POSIX I/O and C library calls in `src/`,
  `modules/` and `bin/` are now checked: a failing pthread call aborts with
  a fatal error (as the checked mutex and condition calls already did), and
  a failing `close()` after `fs::create()` raises an I/O error instead of
  being ignored. A clang-tidy job (`cert-err33-c`,
  `bugprone-unused-return-value`) keeps new unchecked calls out.

### Fixed

- Fix the pthread mutex of `RCLock` (used where spin locks are unavailable,
  such as macOS) silently falling back to a default mutex: the type passed
  to `pthread_mutexattr_settype()` included `PTHREAD_PROCESS_PRIVATE`, which
  made the call fail.
- Fix undefined behavior in full-text tokenization when converting the text
  between UTF-8 and UTF-16 fails; it now raises an internal error.
- Fix the debugger's response queue destroying its mutex while still locked.
- The public `hexbinary_stream.h`, `base64_stream.h` and
  `transcode_stream.h` headers include the standard headers they use, so
  they compile on their own with Clang.
