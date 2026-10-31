### Changed

- Replace the vendored Loki singleton library (`src/util/singleton.h`,
  `src/util/threads.h`) with the C++ standard library. The API item factory is
  still destroyed by `Zorba::shutdown()` and re-created after a new
  initialization; concurrent first calls to `Zorba::getItemFactory()` are now
  synchronized.

### Added

- Add `CONTRIBUTING.md` with build, CI, changelog and review guidelines.
