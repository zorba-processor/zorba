# Container images

Zorba publishes two images to the GitHub Container Registry. Both packages are
public.

| Directory | Image | Published by |
|---|---|---|
| `runtime/` | `ghcr.io/zorba-processor/zorba` | `.github/workflows/release.yml` (release tags), `.github/workflows/images.yml` (master) |
| `builder-latest/` | `ghcr.io/zorba-processor/zorba-builder:latest-toolchain` | `images.yml` (master) |
| `builder/` | `ghcr.io/zorba-processor/zorba-builder:ubuntu-24.04` | `images.yml` (master) |
| `http-test-server/` | not published; built locally and in CI for the HTTP tests | |

## `ghcr.io/zorba-processor/zorba`

The `zorba` command-line tool on Ubuntu 24.04, for `linux/amd64` and
`linux/arm64` (one multi-arch tag; Docker pulls the matching platform).

```bash
docker run --rm ghcr.io/zorba-processor/zorba:edge -r -q '1 + 1'
echo 'for $i in 1 to 3 return $i * $i' > query.xq
docker run --rm -v "$PWD":/work ghcr.io/zorba-processor/zorba:edge -r -f -q query.xq
```

The image also contains the MCP server `zorba-mcp`. The entrypoint runs it when
the first argument is `zorba-mcp`; any other arguments go to `zorba`:

```bash
docker run -i --rm -v "$PWD:/data:ro" ghcr.io/zorba-processor/zorba:edge zorba-mcp --root /data
```

`images.yml` smoke-tests it (`docker/runtime/smoke-test.sh`) before it pushes.
A run of that workflow on a branch other than `master` builds and tests the
image and publishes nothing.

| Tag | Points to | Moves? |
|---|---|---|
| `<version>`, e.g. `5.0.0`, `5.0.0-alpha.1` | That release | No |
| `<major>.<minor>`, e.g. `5.0` | Newest stable release of that minor line | Yes, on each stable patch release |
| `<major>`, e.g. `5` | Newest stable release of that major line | Yes, on each stable release |
| `latest` | Newest stable release | Yes, only on stable releases |
| `edge` | Latest build of `master` | Yes, on every push to master |
| `sha-<short sha>` | A specific `master` commit | No |

Prereleases (`-alpha`, `-beta`, `-rc`) get only their `<version>` tag; they
never move `<major>.<minor>`, `<major>` or `latest`. Until the first stable 5.x
release, `latest` is the last master build published before this tag plan
(images.yml used to move it on every master push); use `edge` or a version tag
instead.

## `ghcr.io/zorba-processor/zorba-builder`

Build environments used by CI (`linux/amd64`). Rebuilt from master; there are
no per-release tags.

| Tag | Contents |
|---|---|
| `latest-toolchain` | Arch Linux with the newest GCC, Clang, CMake and libraries (`builder-latest/`); the linux-amd64 CI jobs |
| `ubuntu-24.04` | Ubuntu 24.04 with GCC 13 (`builder/`); the oldest supported toolchain, as in the linux-arm64 CI job |

Both builder images and the `zorba` runtime image use Xerces-C from the
[zorba-processor/xerces-c](https://github.com/zorba-processor/xerces-c) fork
(branch `zorba-patches`, tag pinned to a commit in each Dockerfile; keep the
three in sync), built from source into `/usr/local`, instead of the
distribution package: stock Xerces-C 3.3.0 and earlier reject valid schemas
that restrict a union type (see `doc/KNOWN_ISSUES.txt`). The runtime image ships
only `libxerces-c-3.3.so`. The macOS CI job (Homebrew) and the CodeQL workflow
(Ubuntu packages) use stock Xerces-C.

The three images also build ICU and libxml2 from the
[zorba-processor/icu](https://github.com/zorba-processor/icu) and
[zorba-processor/libxml2](https://github.com/zorba-processor/libxml2) forks
(branch `zorba-patches`, tags `release-78.3-zorba.1` and `v2.15.4-zorba.1`,
pinned to commits in each Dockerfile; keep the three in sync): stock ICU
crashes and stock libxml2's canonicalization drops content when an allocation
fails (see `doc/KNOWN_ISSUES.txt`). Each process loads one copy of each
library:

- `builder-latest/` builds them at exactly the versions of Arch's `icu` and
  `libxml2` packages and installs them over the packages' files in `/usr`, so
  Arch's libxslt and the Xerces-C fork load the patched copy too. The build
  stops when Arch moves to another version; move the forks first.
- `builder/` and `runtime/` install them in `/usr/local` instead of Ubuntu's
  ICU 74 and libxml2 2.9, which have other sonames; Ubuntu's development
  packages are not installed. `builder/` also builds libxslt 1.1.45 against
  them. The runtime image ships the libraries without Ubuntu's.

Each image records the checksums of the patched libraries in
`/usr/local/share/zorba-patched-libs.sha256`. `scripts/check-patched-libs.sh`
(run by `ci.yml` in the builder images and by `images.yml` in the runtime
image) fails when an executable loads more than one copy of ICU or libxml2 or
a copy that is not the recorded one. The macOS CI job, the CodeQL workflow and
ClusterFuzzLite use stock ICU and libxml2.

## Attestations and SBOMs

Release images carry:

- a BuildKit provenance attestation, attached to the image index by
  `docker buildx` when it pushes;
- a signed SLSA build provenance attestation for the multi-arch image and a
  signed SPDX SBOM attestation for each platform image, created with
  `actions/attest` and stored both in the registry and in the repository's
  attestations.

Verify them with the GitHub CLI:

```bash
gh attestation verify oci://ghcr.io/zorba-processor/zorba:5.0.0-alpha.1 -R zorba-processor/zorba
```

The SBOM files (`zorba-<version>-image-<arch>.spdx.json`) are also attached
to the GitHub Release.

### Why GHCR lists `unknown/unknown`

An attestation is stored in the registry as an extra manifest next to the
platform images. It has no operating system or architecture, so the GHCR
package page lists it as an `unknown/unknown` platform (and attestations
pushed by `actions/attest` appear as extra untagged `sha256-…` versions). This
is expected: `docker pull` and `docker run` ignore these manifests and pick
the `linux/amd64` or `linux/arm64` image.
