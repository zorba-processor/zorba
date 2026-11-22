### Added

- Add multithreaded API tests (`test/api/mt`, ctest label `mt`) for the
  engine built with `-DZORBA_FOR_ONE_THREAD_ONLY=OFF`, a ThreadSanitizer CI
  job that runs them (`tsan-mt.yml`, known races listed in
  `test/sanitizers/tsan-mt.supp`), and a CI build of that configuration with
  warnings as errors (`multithread` preset).
