### Fixed

- Fix a signed integer overflow when a date, time or duration value contains
  a very long run of digits. The value is still rejected with the same
  date/time overflow error as before.
