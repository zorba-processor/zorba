# Contributing to Zorba

## Building and testing

Build with the CMake presets in `CMakePresets.json` (requirements and
platform notes are in `README.md`):

```bash
cmake --workflow --preset full-release-offline   # configure, build, test
```

Other presets: `core-release` (no XML Schema), `full-debug`, `bigint`, the
sanitizer builds `asan`, `tsan` and `tsan-bigint`, and `glibcxx-debug`
(libstdc++ debug mode plus `ZORBA_CONTAINER_CHECKS`, which makes a use of an
invalidated iterator of the store's node vectors abort with a diagnostic).
`docker/builder` and `docker/builder-latest` provide ready-made build
environments.

## Continuous integration

CI runs on GitHub Actions (`.github/workflows/`). `ci.yml` builds and tests
every pull request on Linux x86_64, Linux arm64 and macOS; `sanitizers.yml`
runs the suite under AddressSanitizer, ThreadSanitizer and libstdc++ debug
mode. A pull request is ready for review when its CI is green. Add or adjust
a test with every fix or feature where feasible.

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

**Sanitizer baselines.** `sanitizers.yml` fails a job only on a failure that
`test/baselines/sanitizers-<job>.txt` does not list (`asan`, `tsan`,
`tsan-bigint`, `glibcxx-debug`), so a job with known failures still catches
new ones. QT3 works the same way in the other direction:
`test/fots/qt3-baseline-pass.txt` lists the cases that must pass. Each line is
`test name | kind | reason`: the ctest name, the failure kind ctest prints
(`Failed`, `Subprocess aborted`, `Timeout`, `SEGFAULT`, ...) or `flaky`
(allowed to pass or fail in any way), and the reason (a backlog item or
issue). The job fails on a failure that is not listed and on a listed test
that fails with a different kind. A listed test that passes (or no longer
exists) is a `::warning::` and a "Remove from the baseline" list in the job
summary, not a failure, so fixes merge in any order. A pull request that fixes
a listed test removes its entries from the baseline in the same pull request.

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

## Coding rules

### Owning memory

New and changed code does not manage memory with raw owning `new` and
`delete`. Existing code is not rewritten for this rule; it follows the rule
when it is changed.

Reported (check `zorba-owning-memory`):

- every `delete` and `delete[]` expression;
- every `new` expression whose result is not handed straight to a smart
  pointer: a raw pointer variable, member, return value, container element or
  function argument initialized from `new`.

Allowed:

- `new` passed straight to a smart pointer (`rchandle`, `const_rchandle`,
  `SmartPtr`, `store::ItemHandle` (the base of `store::Item_t`),
  `std::unique_ptr`, `std::shared_ptr`, or a class derived from one):
  construction, copy-initialization, assignment, `reset()`, a return value or
  argument of smart pointer type, `emplace` into a container whose
  `value_type` is a smart pointer. `emplace` into a map is reported (its
  `value_type` is a pair): write `m.emplace(k, std::make_unique<T>())`. For `std::unique_ptr` and `std::shared_ptr`,
  `modernize-make-unique` and `modernize-make-shared` still ask for
  `std::make_unique` and `std::make_shared`.
- placement new: the expression arena (`new (theMemoryMgr) T(...)`,
  `src/compiler/expression/mem_manager.h`) and construction in a buffer.
  `new (std::nothrow)` is an owning `new` and is reported.

Converting:

| Instead of | Write |
|---|---|
| `T* p = new T(a);` ... `delete p;` | `auto p = std::make_unique<T>(a);` |
| member `T* theP;` set with `new`, deleted in the destructor | `std::unique_ptr<T> theP;` |
| `std::vector<T*>` that deletes its elements | `std::vector<std::unique_ptr<T>>` |
| `T* f() { return new T; }` (caller owns) | `std::unique_ptr<T> f() { return std::make_unique<T>(); }` |
| reference-counted `T` (derives from `SimpleRCObject` or `SyncedRCObject`) | `rchandle<T> h(new T);` or `rchandle<T> h = new T;` |
| legacy API that takes ownership of a `T*` | `auto p = std::make_unique<T>(a);` ... `api(p.release());` |

A justified exception (for example a class that deletes itself, or a
third-party interface) carries `// NOLINT(zorba-owning-memory)` with the
reason on the same line.

A `new` or `delete` written in a macro is located where its token is
written: in the macro body, or at the invocation when it is in a macro
argument. A changed line that only invokes a macro whose unchanged body has a
raw `new` is not reported.

The check is a clang-tidy plugin in `scripts/clang-tidy` (see its
`CMakeLists.txt`). `clang-tidy.yml` runs it on every pull request that
touches C or C++ sources, on the lines the pull request adds or changes
relative to the merge base with its base branch, and fails on any finding.
Its scope:

- files under `src/`, `modules/`, `bin/` and `include/`, without the
  generated sources the full clang-tidy run also skips (`*/pregenerated/*`,
  the Bison and Flex output in `src/compiler/parser`);
- only files that a translation unit of the Linux `full-release` build (with
  the debugger) compiles or includes. A changed file outside that build (for
  example a Windows or macOS only source) gets a warning annotation and is
  not checked;
- not `test/` or `bench/`, not the actions in `.y` and `.l` grammar files,
  not code disabled by the preprocessor in that build, not templates that
  are never instantiated;
- whitespace-only changes do not count; moved code counts as changed.

To run it locally on a change (working tree included), with a
`compile_commands.json` from a configured build:

```bash
cmake -S scripts/clang-tidy -B build/clang-tidy-plugin && cmake --build build/clang-tidy-plugin
git diff -U0 --no-color --ignore-space-change $(git merge-base origin/master HEAD) > /tmp/changes.diff
clang-tidy -p build/full-release -load=build/clang-tidy-plugin/ZorbaTidyModule.so \
  --config="{Checks: '-*,zorba-owning-memory', HeaderFilterRegex: '.*', \
    CheckOptions: {zorba-owning-memory.DiffFile: /tmp/changes.diff}}" src/path/to/file.cpp
```

Without `DiffFile` every line is reported.

## License and file headers

Zorba is licensed under the Apache License 2.0 (`LICENSE.txt`). By
submitting a contribution you agree to license it under the same terms.
New source files start with the Apache 2.0 header used by the existing files
(see any file in `src/`). Third-party code must be listed in `doc/legal/NOTICE.xml`.
