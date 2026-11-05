### Fixed

- Fix a signed integer overflow when a date, time or duration value contains
  a very long run of digits. The value is still rejected with the same
  date/time overflow error as before.
- Fix a signed integer overflow when a module version or a `zorba-version`
  option contains a number above 2147483647. Such a number is now treated as
  2147483647.
