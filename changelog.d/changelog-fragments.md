### Added

- Add changelog fragments: each pull request adds `changelog.d/<branch>.md` instead of editing `CHANGELOG.md`, and `scripts/changelog_assemble.py` merges the fragments (with pull request links) into the release section, so open pull requests no longer conflict on the changelog.
