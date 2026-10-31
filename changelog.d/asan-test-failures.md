### Fixed
- Full-text stop-word sets are no longer leaked when a `using stop words at` URI cannot be resolved (FTST0008).

### Changed
- The Sanitizers workflow's asan job now fails when any test fails under ASan/LSan/UBSan instead of only summarizing sanitizer logs.
