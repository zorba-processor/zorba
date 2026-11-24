# Releasing Zorba

Releases are built by `.github/workflows/release.yml` when a `v*` tag is
pushed. The workflow tests on linux-amd64, builds the source tarball, the
binary tarballs, SBOMs, checksums and the multi-arch image, attests them, pushes the image tags
described in `docker/README.md`, and creates a **draft** GitHub Release. Nothing
is public on the Releases page until you publish the draft.

Versions follow SemVer: `vX.Y.Z` for stable releases, `vX.Y.Z-pre` (for
example `v5.0.0-alpha.1`, `v5.0.0-rc.1`) for prereleases. A prerelease is
marked as such on GitHub and does not move the `latest`, `X.Y` or `X` image
tags.

## Steps

1. On master, set the version in `CMakeLists.txt` (`ZORBA_MAJOR_NUMBER`,
   `ZORBA_MINOR_NUMBER`, `ZORBA_PATCH_NUMBER`) to `X.Y.Z`. The workflow fails
   if it does not match the tag.
2. Assemble the changelog. Pull requests add fragments in `changelog.d/`
   instead of editing `CHANGELOG.md` (see `changelog.d/README.md`). In a pull
   request, run:

   ```bash
   python3 scripts/changelog_assemble.py --version X.Y.Z --date YYYY-MM-DD
   ```

   This appends every fragment, with its pull request link, to the matching
   section of `## [X.Y.Z]` (created if missing), and deletes the fragments.
   Use `--dry-run` first to print the result. For a prerelease pass
   `X.Y.Z-pre`; it uses the `[X.Y.Z]` section, and you may omit `--date` to
   keep `Unreleased`. Then edit `CHANGELOG.md` by hand: update the "At a
   glance" table, resolve entries marked **(open PR)**, and for a stable
   release make sure the heading is `## [X.Y.Z] - YYYY-MM-DD`. The release
   notes are taken from this section: the notes of a prerelease list only the
   entries that are not in `CHANGELOG.md` at the previous prerelease tag of the
   same version (the whole section for the first prerelease and for the final
   release). Notes over 120,000 characters link to `CHANGELOG.md` instead (the
   GitHub limit is 125,000).
3. Merge this pull request (and the version change) and wait for CI to pass on master.
4. Optional: rehearse with a dry run, which pushes nothing:

   ```bash
   gh workflow run release.yml -R zorba-processor/zorba --ref master -f version=X.Y.Z-pre -f dry_run=true
   ```

   The dry run uploads the would-be release (tarballs, SBOMs, `SHA256SUMS`,
   notes) as the `release-assets` artifact of the run. Pull requests that
   change `release.yml` run the same dry run automatically. A dry run
   assembles any remaining fragments in the runner only, so the notes preview
   shows them; nothing is pushed to master.

5. Tag the master commit and push the tag:

   ```bash
   git fetch origin
   git tag -a vX.Y.Z-pre -m "Zorba X.Y.Z-pre" origin/master
   git push origin vX.Y.Z-pre
   ```

6. Watch the run: `gh run list -R zorba-processor/zorba --workflow release.yml --limit 1`.
7. Review the draft at https://github.com/zorba-processor/zorba/releases:
   notes, assets (`zorba-X.Y.Z-pre.tar.gz`, the binary tarballs
   `zorba-X.Y.Z-pre-{linux-amd64,linux-arm64,macos-arm64}.tar.xz`, SBOMs,
   `SHA256SUMS`) and the image
   (`docker run --rm ghcr.io/zorba-processor/zorba:X.Y.Z-pre --version`).
8. Publish the draft.

## Binary tarballs

Each binary tarball is the `cmake --install` tree of the `full-release`
preset without the test programs and modules (unit tests, `testdriver`, the
FOTS driver, the util-tests module): `bin/zorba`, `bin/zorba-mcp`,
`include/`, `lib/` and `share/`. The engine is the default build
(single-threaded, `ZORBA_FOR_ONE_THREAD_ONLY=ON`). Zorba looks for its
modules relative to the directory of its library first, so the tree works
wherever it is unpacked.
`scripts/bundle-binary.sh` copies into `lib/` every shared library the tree
loads from outside the system baseline and makes the search paths relative:

- Linux (built in `docker/builder`, Ubuntu 24.04): everything except glibc,
  libstdc++ and libgcc_s is bundled (the patched ICU, libxml2 and Xerces-C,
  libcurl and its libraries, zlib, libuuid); every ELF file gets the RUNPATH
  `$ORIGIN/<path to lib>`. The tarball needs glibc 2.39 and the libstdc++ of
  GCC 13 or later.
- macOS (`macos-15` runner, arm64): ICU, libxml2 and Xerces-C are built from
  the patched forks by `scripts/build-macos-deps.sh` and bundled; libraries in
  `/usr/lib` and `/System` (libcurl, zlib, libiconv) are not. Every library is
  referenced as `@rpath/<name>`, every Mach-O file has the single LC_RPATH
  `@loader_path/<path to lib>`, and every file is signed ad hoc
  (`codesign -s -`): the repository has no signing identity, so a tarball
  downloaded with a browser is quarantined until
  `xattr -dr com.apple.quarantine <dir>` is run.

`scripts/bundle-licenses.sh` puts Zorba's `LICENSE.txt` and `NOTICE.txt` at
the root of the tarball and the licence files of every bundled library in
`share/doc/zorba/third-party/<component>/`: for ICU, libxml2 and Xerces-C
those of the fork at the commit pinned in `docker/builder/Dockerfile`
(`LICENSE`; `Copyright`; `LICENSE` and `NOTICE`), for any other library on
Linux `/usr/share/doc/<package>/copyright` of the Ubuntu package that owns it
(`dpkg -S`). The job fails when a bundled library has no licence file; a
library that is neither a fork nor owned by a package needs a mapping in the
script first.

`scripts/smoke-binary.sh` tests each tarball where none of the bundled
libraries is installed (a plain `ubuntu:24.04` container; on macOS after the
build tree and the dependencies are moved away): the licence files are there,
`zorba` runs an XQuery and a
JSONiq query and a query importing the EXPath file module (a module file
and a module library found relative to `lib/`), `zorba-mcp` answers
`initialize`, and a C++ program compiled against `include/` and linked
against `lib/` runs a query.

## If something fails

- `release.yml` fails on a tag push with "Unassembled changelog fragments"
  when `changelog.d/` still holds fragments (a pull request merged after step
  2, or step 2 skipped). Delete the tag (`git push origin :refs/tags/vX.Y.Z`),
  run step 2 again, merge, and re-tag.
- Before publishing, fix the problem on master, delete the draft release and
  the tag (`git push origin :refs/tags/vX.Y.Z-pre`), and tag again. Image tags
  already pushed for that version are overwritten by the next run.
- To rerun on an existing tag without retagging, run the workflow on the tag:
  `gh workflow run release.yml -R zorba-processor/zorba --ref vX.Y.Z-pre -f dry_run=false`.
  An existing draft is updated in place.
- Never move a tag of a published release; release a new patch version
  instead.
