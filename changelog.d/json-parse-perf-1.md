### Changed

- Make `parse-json` faster and allocate less: the JSON lexer reads characters
  from the stream buffer without a per-character `istream` sentry, clearing a
  shared string no longer allocates, parsed objects reserve their pairs, and
  object keys are first matched against the previous top-level value's keys.
  On a synthetic 213 MB NDJSON log, counting the events takes 3.9 s instead
  of 5.7 s, with 224 allocations per event instead of 415.

### Added

- Add `bench/json-parse/`: a synthetic NDJSON log generator, the benchmark of
  four streaming `parse-json` queries, the profiling helpers, and the results.
