### Changed

- A CI job that stalls now ends after 45 minutes instead of running for
  hours, and the Linux and macOS jobs write the ctest log to a file and print
  it with passed tests removed and each line cut to 1000 characters.
- The sanitizer workflow now fails when UBSan reports undefined behavior at
  one or more sites. Before, those reports only appeared in the summary.
