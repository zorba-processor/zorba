# Directory benchmark: results

## The background scan of zorba-mcp

Run 41462973980 of `.github/workflows/directory-scaling.yml` (`suite=scan`,
`trees=100:2 1000:2 10000:2`, 3 runs per point, medians), commit 8bc7aec87,
ubuntu-24.04, full-release build. Files of about 2 KB from `gen_tree.xq`
(XML, JSON and CSV in turn, 32 x 8 directories), read right after being written,
so from the page cache. `bench/directory/run_scan.sh` runs one server per point:

- `scan`: the default; `cache-1`: `--scan-cache-mb 1`; `no-scan`: `--no-scan`.
- The handshake is measured from the start of the process to the `initialize`
  response. The first `list_documents` comes right after it, with no arguments.
- `scan complete ms` also counts from the start of the process, with
  `list_documents` (limit 0) polled every 2 ms until the state is not `running`.
- With the scan, the first `list_documents` ran before the walker had found any
  file: 0 files and the 32 subdirectories. Its time and size say what the answer
  costs at that moment, not what a listing costs later. With `--no-scan` it is
  `dir:list` over the whole tree.
- RSS at the end is read after the scan ended, or after the first
  `list_documents` with `--no-scan`.



### Cache accounting against RSS

| files | mode | accounted bytes | accounted MiB | RSS growth from handshake to end, MiB |
|---:|---|---:|---:|---:|
| 100 | scan | 105,663 | 0.10 | 0.3 |
| 1,000 | scan | 849,099 | 0.81 | 0.9 |
| 10,000 | scan | 7,992,099 | 7.62 | 5.7 |
| 10,000 | cache-1 | 1,048,564 | 1.00 | 1.2 |

Growth is the medians of the table; the last column is rounded to 0.1 MiB.

- Bytes per entry on this tree: 395 for the file index (2655 files filled 1 MiB
  in the `cache-1` run, with no previews left, directories included) and about
  404 more for a preview and its fields (799 per file in the `scan` run at
  10,000 files). 100 and 1,000 files give 1057 and 849 per file, because the 256
  directories weigh more. Real previews vary: their bound is about 5 KB.
- RSS grows by about 0.3 MiB at 100 files, for almost nothing accounted: two
  thread stacks and allocator arenas, which the accounting does not cover. Nor
  does it cover the buffer of a file being read (at most 1 MiB, transient). So
  at small limits RSS growth can pass the accounted bytes by that amount; at
  10,000 files the accounted bytes are above the growth by 1.3x.
- `--scan-cache-mb` defaults to 64: at 799 bytes per file that holds about 84,000
  files with previews, or about 170,000 in the index alone. At the ratio of
  10,000 files the RSS growth at the limit would be about 48 MiB; that is
  extrapolated, not measured.

These numbers predate the later change to bounded listing and SAX parsing of
HTML, and were not measured again.

Limits: one small-file tree shape on one runner; no cold cache, no slow file
system, no files over the 1 MiB cap.
