### Changed
- Reorganized the repository root: CMake support files moved to `cmake/` (`cmake_modules/` is now `cmake/modules/`, `config/` is now `cmake/config/`, plus the CMake helper `.txt`/`.cmake`/`.in` files); `NOTICE.xml` moved to `doc/legal/`, `KNOWN_ISSUES.txt` to `doc/`, `import_tests.sh` to `scripts/`. Installed CMake files (`ZorbaConfig.cmake`, `ZorbaUse.cmake`, `ZorbaModule.cmake`, `ExternalModuleConfig.cmake.in`) keep their install locations.
- `README.txt` and `Readme.md` merged into `README.md`.

### Removed
- Unused `WIN32.cpack`, `zorba_install.nsi`, the empty `CPack.cmake` stub and the stale `README.txt`.
- Incomplete copy of the Node.js binding (`binding.gyp`, `package.json`, `example.js`, `include/QueryManager.h`, `include/NodeDiagnosticHandler.h`, `include/helpers.h`); its sources were never committed here and the full project lives at github.com/Rod-O/zorba-node. A Node.js binding over the new C API is planned for 5.x.
