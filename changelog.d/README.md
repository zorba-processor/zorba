# Changelog fragments

Pull requests do not edit `CHANGELOG.md`. Each pull request adds one fragment
file here instead, so open pull requests never conflict on the changelog.
`scripts/changelog_assemble.py` merges the fragments into `CHANGELOG.md` at
release time (see `RELEASING.md`).

## File name

`changelog.d/<branch-name>.md`, for example `changelog.d/secure-limits.md`.
Any other `changelog.d/<name>.md` works too; only `README.md` is ignored.
One file per pull request. Edit your own file when the pull request changes.

## Format

Markdown with `### <Section>` headings and `- ` bullets. The section names are
exactly the ones used in `CHANGELOG.md`:

`Breaking changes`, `Security`, `Added`, `Changed`, `Fixed`, `Removed`

```markdown
### Added

- Add the `--frobnicate` option to `zorba` to ...

### Fixed

- Fix a crash in `fn:foo` when ... (long entries may continue on
  indented lines)
```

Rules:

- Do not write pull request links. The assembler finds the pull request
  number from git history (the merge commit that added the file, or the
  `(#N)` suffix of a squash commit) and appends
  ` ([#N](https://github.com/zorba-processor/zorba/pull/N))` to each bullet.
- Use the same style as existing `CHANGELOG.md` entries: one sentence that
  says what changed and, for fixes, what was wrong.
- A pull request with no user-visible change adds no fragment and writes
  `Changelog: none` in its description.
- Check a fragment locally with `python3 scripts/changelog_assemble.py --check`.

## At a glance

The "At a glance" table of `CHANGELOG.md` is not generated from fragments. The
owner edits it by hand when preparing a release.

## Assembling

The owner runs, in a pull request before tagging:

```bash
python3 scripts/changelog_assemble.py --version X.Y.Z --date YYYY-MM-DD
```

Add `--dry-run` to print the resulting `CHANGELOG.md` and change nothing. See
`RELEASING.md` for the full flow.
