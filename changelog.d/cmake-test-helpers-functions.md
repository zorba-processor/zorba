### Fixed

- Fix test arguments with a backslash being passed differently by CMake before and
  after 4.4 (policy CMP0219): `ZORBA_ADD_TEST`, `ZORBA_SECURITY_TEST` and
  `ZORBA_SECURITY_DENIED` are functions now, so every CMake version passes the
  arguments to CTest unchanged. `ZORBA_ADD_TEST` no longer reads the `ARGVn` of
  its caller.
