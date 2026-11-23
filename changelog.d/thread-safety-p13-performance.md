### Added

- Add `bench/threads/`, a multithreaded throughput benchmark: N threads run
  clones of one query against one engine and store, and it reports queries
  per second and scaling per thread count.

### Changed

- Moving an item handle no longer changes the item's reference count, and
  the JSON loader moves the items it parses into place: `parse-json` is
  faster in both builds, most in the multithreaded build, where each count
  change is an atomic operation.
- In the engine built with `-DZORBA_FOR_ONE_THREAD_ONLY=OFF`, the `zorba::Item`
  methods no longer take the store lock for an atomic item, which no update
  changes.
