### Added

- Add an opt-in to run the W3C QT3 suite under the secure policy (`test/fots/qt3_run.py --policy secure`, QT3 workflow input `policy`): the driver and the test-case queries get only file-read under the suite, driver, manifest and expected-failures directories plus native modules; the default run is unchanged.
