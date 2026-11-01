### Breaking changes
- Windows is no longer a supported platform. Supported platforms are Linux (amd64, arm64) and macOS (arm64). Configuring on a non-UNIX system now stops with an error. The .NET (C#) bindings are not affected.

### Removed
- Windows-only build files and helpers: `cmake/modules/Windows/` (proxy find modules, DLL version resource), the Visual Studio project template, `scripts/win_exec.bat.in`, NSIS CPack options, the `.chm` documentation target, the bundled `modules/util-curl/cacert.pem`, and the MSVC/WinCE/Cygwin/MinGW branches of the CMake files.
- Windows-only sources and branches: `src/diagnostics/StackWalker.{h,cpp}`, `src/zorbamisc/config/{win32,cygwin,compatib_defs,stdint}.h`, and every `WIN32`/`_WIN32`/`WINCE`/`CYGWIN`/`_MSC_VER` code path in `src/`, `include/`, `bin/`, `modules/` and the tests, including the Win32 `Latch`/`Condition`, `LoadLibrary` module loading and the Windows `fs_util` functions.
- The `stdafx.h` includes (576 files) and the unused MSVC precompiled-header option `ZORBA_PRECOMPILED_HEADERS`; the real precompiled header (`ZORBA_USE_PCH`) is unchanged.
- The Windows build page `doc/zorba/build_windows.dox` and the Windows notes in the manual.
