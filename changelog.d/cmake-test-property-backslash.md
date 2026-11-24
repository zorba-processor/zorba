### Fixed

- Fix test regular expressions with a backslash being checked differently by CMake
  before and after 4.4 (policy CMP0219): `ZORBA_SET_TEST_PROPERTY` and the test
  helpers in `bin/CMakeLists.txt` that forward a regular expression are functions
  now, so a backslash reaches CTest unchanged on every CMake version. The tests
  `bin/zorba_r_one_plus_one` and `bin/zorba_jnse0012` were affected.
