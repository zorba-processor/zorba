# Contributing to Zorba

## Building and testing

Build with the CMake presets in `CMakePresets.json` (requirements and
platform notes are in `README.md`):

```bash
cmake --workflow --preset full-release-offline   # configure, build, test
```

Other presets: `core-release` (no XML Schema), `full-debug`, `bigint`, and
the sanitizer builds `asan`, `tsan` and `tsan-bigint`. `docker/builder` and
`docker/builder-latest` provide ready-made build environments.

## Continuous integration

CI runs on GitHub Actions (`.github/workflows/`). `ci.yml` builds and tests
every pull request on Linux x86_64, Linux arm64 and macOS; `sanitizers.yml`
runs the suite under AddressSanitizer and ThreadSanitizer. A pull request is
ready for review when its CI is green. Add or adjust a test with every fix
or feature where feasible.

**Quick check.** `quick-check.yml` builds one configuration and runs only
the tests you name, in a few minutes with a warm ccache, instead of the full
CI (8 jobs). It is dispatched by hand, never replaces or weakens a CI job and
is not a required check. It can run on a branch with an open pull request.
The workflow file must be on `master` before it can be dispatched.

```bash
gh workflow run quick-check.yml --ref my-branch \
  -f tests='^test/rbkt/Queries/zorba/fn/' [-f asan=true]
gh run list --workflow quick-check.yml --branch my-branch --limit 1
gh run watch <run-id> --interval 60 --exit-status
```

Inputs: `tests` (ctest `-R` regex; empty builds only; a regex that matches
no test fails), `preset` (default `full-release`, the linux-amd64 job of
`ci.yml`), `asan` (build the `asan` preset instead), `ctest_args`,
`test_timeout` (seconds per test, default 300) and `save_cache` (keep this
run's ccache for later quick checks of the branch; off by default because the
repository cache quota is full). The job summary has the ctest summary and
the truncated output of failed tests; the artifact `quick-check-log` has the
full log. The final push of a pull request still needs the full CI.

## Pull requests

- One commit per logical change. The commit message says why the change is
  made, not only what it does.
- Keep the public API (`include/zorba/`) source-compatible unless the change
  is about the API.
- Do not edit `CHANGELOG.md`. Add one fragment file
  `changelog.d/<branch-name>.md` instead; the format is described in
  `changelog.d/README.md`. A pull request without a user-visible change adds
  no fragment and says `Changelog: none` in its description.

**Review comments.** State proposals directly: what should change, and why. A pull request is the preferred form. Questions are welcome when you need information, not as a way to suggest a change.

## License and file headers

Zorba is licensed under the Apache License 2.0 (`LICENSE.txt`). By
submitting a contribution you agree to license it under the same terms.
New source files start with the Apache 2.0 header used by the existing files
(see any file in `src/`). Third-party code must be listed in `doc/legal/NOTICE.xml`.
