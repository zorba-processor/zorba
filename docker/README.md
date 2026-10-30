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
docker run --rm ghcr.io/zorba-processor/zorba:edge -q '1 + 1'
docker run --rm -v "$PWD":/work ghcr.io/zorba-processor/zorba:edge -f -q query.xq
```

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
that restrict a union type (see `KNOWN_ISSUES.txt`). The runtime image ships
only `libxerces-c-3.3.so`. The macOS CI job (Homebrew) and the CodeQL workflow
(Ubuntu packages) use stock Xerces-C.

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
