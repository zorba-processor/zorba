### Fixed

- The `zorba` image and the builder images build ICU and libxml2 from patched
  forks (zorba-processor/icu `release-78.3-zorba.1`, zorba-processor/libxml2
  `v2.15.4-zorba.1`). With them, a failed allocation while ICU compiles a
  regular expression is reported as an out-of-memory error instead of
  crashing, and `x:canonicalize()` reports a failed allocation instead of
  returning a canonical form that lacks a namespace declaration, an attribute
  or an attribute's value. Each process loads one copy of each library.

### Changed

- The `zorba` image and the Ubuntu 24.04 builder image use ICU 78.3 and
  libxml2 2.15.4, in `/usr/local`, instead of Ubuntu's ICU 74 and libxml2
  2.9.14.
