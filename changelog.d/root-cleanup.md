### Changed
- Reorganized the repository root: CMake support files moved to `cmake/` (`cmake_modules/` is now `cmake/modules/`, `config/` is now `cmake/config/`, plus the CMake helper `.txt`/`.cmake`/`.in` files); `NOTICE.xml` moved to `doc/legal/`, `KNOWN_ISSUES.txt` to `doc/`, `import_tests.sh` to `scripts/`. Installed CMake files (`ZorbaConfig.cmake`, `ZorbaUse.cmake`, `ZorbaModule.cmake`, `ExternalModuleConfig.cmake.in`) keep their install locations.
- `README.txt` and `Readme.md` merged into `README.md`.

### Removed
- Unused `WIN32.cpack`, `zorba_install.nsi`, `binding.gyp`, `package.json`, `example.js`, the Node.js binding headers `include/QueryManager.h`, `include/NodeDiagnosticHandler.h` and `include/helpers.h`, the empty `CPack.cmake` stub and the stale `README.txt`.
