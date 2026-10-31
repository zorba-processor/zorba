### Changed
- CI: the embedded, embedded-noicu, bigint and full-nopch profile jobs now run on a pull request only when files in their area change (path filters in `.github/ci-filters.yml`); the required jobs still run for any code change and master pushes run everything. `scripts/check-profile-filters.sh` fails if a file containing a profile's guard macro is not covered by that profile's filter.
