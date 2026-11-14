### Fixed
- The hardening option detects AddressSanitizer flags given for a build type in mixed case (for example `RelWithDebInfo`), so it no longer combines hardening with ASan in that case.
