# Semi-structured time series through zorba-mcp

Measured on GitHub Actions `ubuntu-24.04` runners, Release build (`full-release`,
Arch builder image), 2026-11-03. Every cell goes through `run_query` of `zorba-mcp`.

| engine | commit | run | jobs: size, CPU |
|---|---|---|---|
| master before PR #169 | `7c8d649f97` | 41659837513 | 100 MB: EPYC 7763; 1 GB: EPYC 9V45; 10 GB: EPYC 7763 |
| `window-performance`, the head of PR #169 | `eabbb66ea3` | 41659857371 | 100 MB: EPYC 7763; 1 GB: EPYC 9V74; 10 GB: EPYC 9V45 |

PR #169 was merged into master as `e1138396f4` after these runs started. In this file,
"master" is `7c8d649f97`, without it; `window-performance` is what it merged.

Each job has its own runner, so times of different jobs come from different CPUs.
Every runner: 4 vCPU, 16760344576 bytes of RAM (15.6 GiB), 3221221376 bytes of swap,
154894188544 bytes of disk with about 89 GB free, no cgroup memory limit in the
container, job limit 355 minutes. The runner holds one form of the data at a time.

## What is measured

Each cell is one query: `prolog.jq`, the input expression of the access path, and
`ops/<operation>.jq`. It is one `run_query` call in a fresh `zorba-mcp` session over
a root that holds only that form, driven by `bench/directory/dirbench.cpp`.

- **Time** is the call, from the request until the response or until the session
  closes. It does not include starting the server. The files are in the page cache,
  just written.
- **Peak RSS** is that of the server from `wait4`, with the background scan running.
- **Default limits**: no option but `--root`, so `--timeout 30` and `--max-memory
  4294967296` (`RLIMIT_AS`), and the default scan and document cache. With one call
  per session the document cache serves nothing.
- **Extended limits**: `--timeout 1200 --max-memory 15032385536` (14 GiB). Each cell
  that fails with the default limits runs once more with these. They are not defaults.
- **Runs**: at 100 MB the median of 3 runs and the highest RSS; at 1 GB and 10 GB one
  run. A failed run is not repeated.
- The query ends in one number, so the result takes no memory. For each operation and
  size, every cell that gave a result gave the same one, on all forms and access paths.

Run with `bench/scale/run.sh`, or the suite `scale` of
`.github/workflows/directory-scaling.yml`. Raw data per cell: `cells.tsv` in the
artifacts `scale-<size>` of each run, for as long as GitHub keeps them.

### Data

`gen.cpp` writes the same records in every form; its header gives the record. They are
irregular: nested `metrics` and `meta`, optional `v` (absent, null, string or number),
optional `metrics.temp`, `extra` and `note`. Timestamps go up by 200 to 1800 ms, and by
10 minutes about every 5000 records. The size is that of the JSON Lines text of the
records, in 10^6 bytes.

| size | records | jsonl, dir bytes | array bytes | csv bytes | dir files |
|---|---:|---:|---:|---:|---:|
| 100 MB | 646836 | 100000016 | 100646855 | 43185236 | 1294 |
| 1 GB | 6427200 | 1000000039 | 1006427242 | 435541251 | 12855 |
| 10 GB | 63861356 | 10000000134 | 10063861493 | 4391338306 | 127723 |

Forms: `jsonl`, JSON Lines in one file; `array`, one JSON array in one file; `dir`, JSON
Lines files of 500 records (about 77 KB) in one directory; `csv`, one file with the
objects flattened (`metrics.temp` is `metrics_temp`, no `meta.tags`).

### Access paths

| input | expression |
|---|---|
| `*-open` | `dir:open($dir:root, "data.<ext>").content[]` |
| `*-documents` | `dir:documents($dir:root).content[]` |
| `jsonl-direct` | `parse-json(file:read-text(F))` |
| `array-direct` | `parse-json(file:read-text(F))[]` |
| `csv-direct` | `csv:parse(file:read-text(F), { "cast-letter-booleans" : false })` |
| `dir-direct` | `for $f in file:list($dir:root) order by $f return parse-json(file:read-text(...))` |
| `jsonl-lines` | `file:read-text-lines(F) ! parse-json($$)` |
| `dir-lines` | `for $f in file:list($dir:root) order by $f return file:read-text-lines(...) ! parse-json($$)` |

`dir:open` takes one file, so `dir` has no `open`. `array` and `csv` have no line-based
path: their lines are not items of their own.

### Operations

| operation | query, over all records |
|---|---|
| `filter` | number of records with `metrics.temp` above 90 |
| `aggregate` | average of the numeric `v` |
| `group_by` | average of `v` per `node` (64 groups), summed |
| `order_by` | all records by `metrics.temp` descending, `seq` of the first |
| `join` | equality on `node` with a table of 64 rows written in the query, sum of `rack` |
| `tumbling` | `tumbling window`, 1 minute from its first record, average of `v`, summed |
| `sliding_count` | `sliding window`, 10 records from each record, average of `v`, summed |
| `sliding_time` | `sliding window`, 1 minute from each record, average of `v`, summed |

### How a query stops

| mark | what the client gets | text |
|---|---|---|
| none | the result | |
| `T` | tool error `zerr:ZXQP0068` | `query timeout exceeded: 30000 ms` (`1200000 ms` with the extended limits) |
| `M` | tool error `memory_limit` | `the query ran out of memory (server limit --max-memory: 4294967296 bytes); narrow the query, or start the server with a higher limit` (`15032385536 bytes` with the extended limits) |
| `C` | no response: the server process ended during the call and the session closed | |
| `NR` | not run: the job's budget of 5 hours of measuring ran out | |
| `–` | not run: the cell ended with the default limits | |

A time limit stopped queries between 30.0 and 33.3 s with the default limits, and at
1200.0 s with the extended ones. The memory limit stops a query at about 3880 MiB of RSS
with the default limit and about 14090 MiB with the extended one. With the same query,
input and limits, a run may end in `M` or in `C`. A `C` cell takes about twice
as long as an `M` cell of the same input; time in `C` runs until the session closes,
not until the failed allocation.

## Largest size that gives a result, master

With the default limits:

| input | filter | aggregate | group_by | order_by | join | tumbling | sliding_count | sliding_time |
|---|---|---|---|---|---|---|---|---|
| jsonl-open | 100 MB | 100 MB | 100 MB | 100 MB | 100 MB | 100 MB | 100 MB | < 100 MB |
| jsonl-documents | 100 MB | 100 MB | 100 MB | 100 MB | 100 MB | 100 MB | 100 MB | < 100 MB |
| jsonl-direct | 1 GB | 1 GB | 100 MB | 100 MB | 100 MB | 100 MB | 100 MB | < 100 MB |
| jsonl-lines | 100 MB | 100 MB | 100 MB | 100 MB | 100 MB | 100 MB | 100 MB | < 100 MB |
| array-open | 100 MB | 100 MB | 100 MB | 100 MB | 100 MB | 100 MB | 100 MB | < 100 MB |
| array-documents | 100 MB | 100 MB | 100 MB | 100 MB | 100 MB | 100 MB | 100 MB | < 100 MB |
| array-direct | 100 MB | 100 MB | 100 MB | 100 MB | 100 MB | 100 MB | 100 MB | < 100 MB |
| dir-documents | 100 MB | 100 MB | 100 MB | 100 MB | 100 MB | 100 MB | 100 MB | < 100 MB |
| dir-direct | 100 MB | 100 MB | 100 MB | 100 MB | 100 MB | 100 MB | 100 MB | < 100 MB |
| dir-lines | 100 MB | 100 MB | 100 MB | 100 MB | 100 MB | 100 MB | 100 MB | < 100 MB |
| csv-open | 100 MB | 100 MB | 100 MB | 100 MB | 100 MB | 100 MB | 100 MB | < 100 MB |
| csv-documents | 100 MB | 100 MB | 100 MB | 100 MB | 100 MB | 100 MB | 100 MB | < 100 MB |
| csv-direct | 1 GB | 1 GB | 1 GB | 100 MB | 1 GB | 1 GB | 100 MB | < 100 MB |

With the extended limits, for the cells that needed them:

| input | filter | aggregate | group_by | order_by | join | tumbling | sliding_count | sliding_time |
|---|---|---|---|---|---|---|---|---|
| jsonl-open | 1 GB | 1 GB | 1 GB | 1 GB; 10 GB not run | 1 GB; 10 GB not run | 1 GB; 10 GB not run | 1 GB; 10 GB not run | 1 GB; 10 GB not run |
| jsonl-documents | 1 GB | 1 GB | 1 GB | 1 GB; 10 GB not run | 1 GB; 10 GB not run | 1 GB; 10 GB not run | 1 GB; 10 GB not run | 1 GB; 10 GB not run |
| jsonl-direct | 10 GB | 10 GB | 10 GB | 1 GB; 10 GB not run | 1 GB; 10 GB not run | 1 GB; 10 GB not run | 1 GB; 10 GB not run | 1 GB; 10 GB not run |
| jsonl-lines | 10 GB | 10 GB | 1 GB; 10 GB not run | 1 GB; 10 GB not run | 1 GB; 10 GB not run | 1 GB; 10 GB not run | 1 GB; 10 GB not run | 1 GB; 10 GB not run |
| array-open | 1 GB | 1 GB | 1 GB | 1 GB | 1 GB | 1 GB | 1 GB | 1 GB |
| array-documents | 1 GB | 1 GB | 1 GB | 1 GB | 1 GB | 1 GB | 1 GB | 1 GB |
| array-direct | 1 GB | 1 GB | 1 GB | 1 GB | 1 GB | 1 GB | 1 GB | 1 GB |
| dir-documents | 10 GB | 10 GB | 10 GB | 1 GB; 10 GB not run | 1 GB; 10 GB not run | 1 GB; 10 GB not run | 1 GB; 10 GB not run | 1 GB; 10 GB not run |
| dir-direct | 1 GB | 1 GB | 1 GB; 10 GB not run | 1 GB; 10 GB not run | 1 GB; 10 GB not run | 1 GB; 10 GB not run | 1 GB; 10 GB not run | 1 GB; 10 GB not run |
| dir-lines | 10 GB | 10 GB | 1 GB; 10 GB not run | 1 GB; 10 GB not run | 1 GB; 10 GB not run | 1 GB; 10 GB not run | 1 GB; 10 GB not run | 1 GB; 10 GB not run |
| csv-open | 1 GB | 1 GB | 1 GB | 1 GB | 1 GB | 1 GB; 10 GB not run | 1 GB; 10 GB not run | 1 GB; 10 GB not run |
| csv-documents | 1 GB | 1 GB | 1 GB | 1 GB | 1 GB | 1 GB; 10 GB not run | 1 GB; 10 GB not run | 1 GB; 10 GB not run |
| csv-direct | 10 GB | 10 GB | 10 GB | 1 GB | 10 GB | 1 GB; 10 GB not run | 1 GB; 10 GB not run | 1 GB; 10 GB not run |

## Largest size that gives a result, `window-performance`

Window operations only. With the default limits:

| input | tumbling | sliding_count | sliding_time |
|---|---|---|---|
| jsonl-open | 100 MB | 100 MB | < 100 MB |
| jsonl-documents | 100 MB | 100 MB | < 100 MB |
| jsonl-direct | 100 MB | 100 MB | < 100 MB |
| jsonl-lines | 100 MB | 100 MB | < 100 MB |
| array-open | 100 MB | 100 MB | < 100 MB |
| array-documents | 100 MB | 100 MB | < 100 MB |
| array-direct | 100 MB | 100 MB | < 100 MB |
| dir-documents | 100 MB | 100 MB | < 100 MB |
| dir-direct | 100 MB | 100 MB | < 100 MB |
| dir-lines | 100 MB | 100 MB | < 100 MB |
| csv-open | 100 MB | 100 MB | < 100 MB |
| csv-documents | 100 MB | 100 MB | < 100 MB |
| csv-direct | 1 GB | 100 MB | < 100 MB |

With the extended limits, for the cells that needed them:

| input | tumbling | sliding_count | sliding_time |
|---|---|---|---|
| jsonl-open | 1 GB | 1 GB | 1 GB |
| jsonl-documents | 1 GB | 1 GB | 1 GB |
| jsonl-direct | 10 GB | 10 GB | 1 GB |
| jsonl-lines | 10 GB | 10 GB | 1 GB |
| array-open | 1 GB | 1 GB | 1 GB |
| array-documents | 1 GB | 1 GB | 1 GB |
| array-direct | 1 GB | 1 GB | 1 GB |
| dir-documents | 10 GB | 10 GB | 1 GB |
| dir-direct | 1 GB | 1 GB | 1 GB |
| dir-lines | 10 GB | 10 GB | 1 GB |
| csv-open | 1 GB | 1 GB | 1 GB |
| csv-documents | 1 GB | 1 GB | 1 GB |
| csv-direct | 10 GB | 10 GB | 1 GB |

## 100 MB, master

Default limits:

| input | filter | aggregate | group_by | order_by | join | tumbling | sliding_count | sliding_time |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| jsonl-open | 7.2 s, 1399 MiB | 7.2 s, 1399 MiB | 7.7 s, 1399 MiB | 10.6 s, 1399 MiB | 8.7 s, 1400 MiB | 8.1 s, 1399 MiB | 10.7 s, 1399 MiB | T 30.7 s, 1399 MiB |
| jsonl-documents | 7.3 s, 1399 MiB | 7.2 s, 1399 MiB | 7.7 s, 1399 MiB | 10.6 s, 1399 MiB | 8.7 s, 1400 MiB | 8.1 s, 1400 MiB | 10.6 s, 1399 MiB | T 30.7 s, 1399 MiB |
| jsonl-direct | 4.7 s, 37 MiB | 4.5 s, 37 MiB | 5.1 s, 83 MiB | 8.1 s, 249 MiB | 6.1 s, 38 MiB | 5.8 s, 38 MiB | 8.7 s, 813 MiB | T 30.3 s, 815 MiB |
| jsonl-lines | 5.9 s, 37 MiB | 5.9 s, 37 MiB | 6.1 s, 83 MiB | 9.6 s, 249 MiB | 7.5 s, 38 MiB | 7.0 s, 38 MiB | 10.4 s, 1432 MiB | T 30.7 s, 1434 MiB |
| array-open | 6.5 s, 1113 MiB | 6.4 s, 1113 MiB | 7.0 s, 1118 MiB | 10.4 s, 1234 MiB | 7.9 s, 1114 MiB | 7.3 s, 1114 MiB | 9.8 s, 1119 MiB | T 30.8 s, 1120 MiB |
| array-documents | 6.5 s, 1113 MiB | 6.4 s, 1114 MiB | 6.9 s, 1118 MiB | 10.3 s, 1234 MiB | 7.9 s, 1114 MiB | 7.3 s, 1114 MiB | 9.9 s, 1119 MiB | T 30.8 s, 1120 MiB |
| array-direct | 5.5 s, 826 MiB | 5.3 s, 826 MiB | 5.8 s, 831 MiB | 9.3 s, 946 MiB | 6.8 s, 826 MiB | 6.2 s, 826 MiB | 8.9 s, 826 MiB | T 30.3 s, 828 MiB |
| dir-documents | 6.4 s, 39 MiB | 6.3 s, 39 MiB | 7.1 s, 89 MiB | 10.7 s, 267 MiB | 7.8 s, 40 MiB | 7.1 s, 40 MiB | 10.4 s, 828 MiB | T 30.5 s, 829 MiB |
| dir-direct | 5.3 s, 816 MiB | 5.2 s, 816 MiB | 5.7 s, 820 MiB | 9.0 s, 856 MiB | 6.7 s, 816 MiB | 6.1 s, 816 MiB | 9.0 s, 827 MiB | T 30.4 s, 827 MiB |
| dir-lines | 6.0 s, 37 MiB | 5.8 s, 37 MiB | 6.3 s, 84 MiB | 9.7 s, 250 MiB | 7.6 s, 38 MiB | 7.0 s, 37 MiB | 10.5 s, 1433 MiB | T 30.7 s, 1434 MiB |
| csv-open | 4.3 s, 944 MiB | 4.2 s, 944 MiB | 4.6 s, 944 MiB | 7.5 s, 975 MiB | 5.8 s, 944 MiB | 5.1 s, 944 MiB | 7.9 s, 944 MiB | T 30.2 s, 944 MiB |
| csv-documents | 4.2 s, 944 MiB | 4.3 s, 944 MiB | 4.6 s, 945 MiB | 7.5 s, 975 MiB | 5.8 s, 944 MiB | 5.1 s, 944 MiB | 7.9 s, 944 MiB | T 30.2 s, 944 MiB |
| csv-direct | 2.5 s, 37 MiB | 2.5 s, 37 MiB | 2.9 s, 83 MiB | 5.7 s, 249 MiB | 4.2 s, 38 MiB | 3.7 s, 38 MiB | 6.8 s, 709 MiB | T 30.2 s, 711 MiB |

Extended limits:

| input | filter | aggregate | group_by | order_by | join | tumbling | sliding_count | sliding_time |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| jsonl-open | – | – | – | – | – | – | – | 70.7 s, 1399 MiB |
| jsonl-documents | – | – | – | – | – | – | – | 70.7 s, 1399 MiB |
| jsonl-direct | – | – | – | – | – | – | – | 68.8 s, 813 MiB |
| jsonl-lines | – | – | – | – | – | – | – | 71.1 s, 1432 MiB |
| array-open | – | – | – | – | – | – | – | 70.0 s, 1119 MiB |
| array-documents | – | – | – | – | – | – | – | 69.8 s, 1119 MiB |
| array-direct | – | – | – | – | – | – | – | 68.8 s, 826 MiB |
| dir-documents | – | – | – | – | – | – | – | 70.6 s, 827 MiB |
| dir-direct | – | – | – | – | – | – | – | 69.1 s, 827 MiB |
| dir-lines | – | – | – | – | – | – | – | 71.1 s, 1433 MiB |
| csv-open | – | – | – | – | – | – | – | 69.1 s, 944 MiB |
| csv-documents | – | – | – | – | – | – | – | 68.2 s, 943 MiB |
| csv-direct | – | – | – | – | – | – | – | 67.0 s, 709 MiB |

## 1 GB, master

Default limits:

| input | filter | aggregate | group_by | order_by | join | tumbling | sliding_count | sliding_time |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| jsonl-open | C 38.6 s, 3878 MiB | M 16.4 s, 3879 MiB | C 34.4 s, 3878 MiB | C 34.7 s, 3877 MiB | C 34.8 s, 3878 MiB | C 34.3 s, 3878 MiB | M 15.9 s, 3879 MiB | C 34.7 s, 3878 MiB |
| jsonl-documents | M 16.1 s, 3879 MiB | C 33.8 s, 3878 MiB | M 15.8 s, 3879 MiB | M 16.1 s, 3879 MiB | M 16.1 s, 3879 MiB | M 16.0 s, 3879 MiB | M 16.5 s, 3879 MiB | C 35.0 s, 3878 MiB |
| jsonl-direct | 28.7 s, 37 MiB | 27.8 s, 38 MiB | T 31.4 s, 504 MiB | T 32.0 s, 2140 MiB | T 30.0 s, 39 MiB | T 30.0 s, 39 MiB | M 16.0 s, 3878 MiB | M 15.9 s, 3878 MiB |
| jsonl-lines | T 30.0 s, 39 MiB | T 30.0 s, 39 MiB | T 31.1 s, 421 MiB | T 32.4 s, 1727 MiB | T 30.0 s, 39 MiB | T 30.0 s, 39 MiB | C 29.3 s, 3883 MiB | C 29.5 s, 3882 MiB |
| array-open | C 34.1 s, 3878 MiB | M 15.6 s, 3879 MiB | M 15.6 s, 3879 MiB | C 33.1 s, 3877 MiB | M 15.1 s, 3879 MiB | C 34.2 s, 3878 MiB | M 15.5 s, 3879 MiB | C 33.4 s, 3878 MiB |
| array-documents | C 33.3 s, 3877 MiB | C 33.1 s, 3878 MiB | M 15.4 s, 3879 MiB | M 15.2 s, 3879 MiB | M 15.4 s, 3879 MiB | M 15.2 s, 3879 MiB | M 16.0 s, 3879 MiB | C 33.3 s, 3878 MiB |
| array-direct | C 33.2 s, 3877 MiB | C 34.0 s, 3877 MiB | M 15.9 s, 3878 MiB | M 15.7 s, 3878 MiB | M 15.6 s, 3879 MiB | C 34.0 s, 3877 MiB | C 33.7 s, 3877 MiB | M 15.8 s, 3878 MiB |
| dir-documents | T 30.0 s, 52 MiB | T 30.0 s, 53 MiB | T 31.0 s, 399 MiB | T 32.8 s, 1754 MiB | T 30.0 s, 53 MiB | T 30.0 s, 53 MiB | M 20.7 s, 3886 MiB | M 20.7 s, 3886 MiB |
| dir-direct | C 35.2 s, 3892 MiB | M 16.2 s, 3894 MiB | C 34.7 s, 3892 MiB | C 34.0 s, 3893 MiB | C 32.1 s, 3893 MiB | C 32.9 s, 3893 MiB | M 15.9 s, 3894 MiB | M 16.0 s, 3894 MiB |
| dir-lines | T 30.0 s, 49 MiB | T 30.0 s, 50 MiB | T 31.1 s, 431 MiB | T 32.4 s, 1752 MiB | T 30.0 s, 50 MiB | T 30.0 s, 50 MiB | M 12.2 s, 3891 MiB | C 27.1 s, 3891 MiB |
| csv-open | C 27.7 s, 3882 MiB | C 26.9 s, 3882 MiB | C 26.8 s, 3882 MiB | C 27.2 s, 3882 MiB | C 27.0 s, 3882 MiB | C 27.1 s, 3883 MiB | C 27.1 s, 3882 MiB | C 27.2 s, 3882 MiB |
| csv-documents | C 10.9 s, 3881 MiB | C 10.5 s, 3881 MiB | C 26.7 s, 3882 MiB | C 26.5 s, 3882 MiB | C 27.0 s, 3882 MiB | C 26.8 s, 3882 MiB | C 26.8 s, 3882 MiB | C 26.4 s, 3882 MiB |
| csv-direct | 14.4 s, 37 MiB | 14.6 s, 37 MiB | 18.1 s, 503 MiB | T 33.3 s, 2152 MiB | 22.9 s, 38 MiB | 22.1 s, 37 MiB | M 10.5 s, 3881 MiB | M 10.4 s, 3881 MiB |

Extended limits:

| input | filter | aggregate | group_by | order_by | join | tumbling | sliding_count | sliding_time |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| jsonl-open | 55.7 s, 13516 MiB | 56.2 s, 13515 MiB | 60.6 s, 13516 MiB | 103.1 s, 13516 MiB | 62.7 s, 13516 MiB | 60.8 s, 13516 MiB | 75.5 s, 13516 MiB | 393.0 s, 13516 MiB |
| jsonl-documents | 55.7 s, 13516 MiB | 56.8 s, 13516 MiB | 61.7 s, 13516 MiB | 102.6 s, 13516 MiB | 63.0 s, 13517 MiB | 59.5 s, 13516 MiB | 76.0 s, 13516 MiB | 382.9 s, 13516 MiB |
| jsonl-lines | 36.4 s, 37 MiB | 36.7 s, 37 MiB | 39.7 s, 503 MiB | 82.9 s, 2153 MiB | 43.4 s, 37 MiB | 42.7 s, 37 MiB | 71.2 s, 13905 MiB | 388.8 s, 13906 MiB |
| jsonl-direct | – | – | 33.2 s, 503 MiB | 74.5 s, 2151 MiB | 35.4 s, 37 MiB | 35.7 s, 37 MiB | 57.4 s, 7755 MiB | 372.1 s, 7755 MiB |
| array-open | 48.6 s, 10668 MiB | 49.0 s, 10667 MiB | 53.4 s, 10753 MiB | 94.1 s, 11928 MiB | 56.8 s, 10668 MiB | 54.2 s, 10668 MiB | 69.4 s, 10717 MiB | 379.5 s, 10717 MiB |
| array-documents | 48.7 s, 10668 MiB | 49.1 s, 10668 MiB | 53.1 s, 10754 MiB | 95.5 s, 11928 MiB | 57.0 s, 10668 MiB | 53.1 s, 10668 MiB | 67.9 s, 10717 MiB | 382.4 s, 10717 MiB |
| array-direct | 37.2 s, 7819 MiB | 37.1 s, 7819 MiB | 41.1 s, 7903 MiB | 83.2 s, 9078 MiB | 45.3 s, 7820 MiB | 43.4 s, 7819 MiB | 59.2 s, 7819 MiB | 381.7 s, 7819 MiB |
| dir-documents | 41.0 s, 51 MiB | 39.6 s, 51 MiB | 45.7 s, 543 MiB | 88.0 s, 2319 MiB | 47.9 s, 51 MiB | 45.1 s, 51 MiB | 67.9 s, 7894 MiB | 391.6 s, 7894 MiB |
| dir-direct | 35.8 s, 7786 MiB | 35.1 s, 7786 MiB | 40.6 s, 7864 MiB | 90.0 s, 8073 MiB | 43.6 s, 7786 MiB | 41.3 s, 7786 MiB | 59.9 s, 7853 MiB | 373.8 s, 7854 MiB |
| dir-lines | 36.0 s, 48 MiB | 35.6 s, 47 MiB | 39.4 s, 514 MiB | 82.0 s, 2164 MiB | 44.5 s, 48 MiB | 42.9 s, 48 MiB | 73.3 s, 13917 MiB | 390.7 s, 13917 MiB |
| csv-open | 30.2 s, 8992 MiB | 30.2 s, 8992 MiB | 32.9 s, 9028 MiB | 73.0 s, 9188 MiB | 38.9 s, 8993 MiB | 37.1 s, 8992 MiB | 53.4 s, 8993 MiB | 389.6 s, 8992 MiB |
| csv-documents | 30.1 s, 8992 MiB | 29.7 s, 8992 MiB | 33.8 s, 9029 MiB | 71.8 s, 9189 MiB | 39.6 s, 8993 MiB | 36.0 s, 8992 MiB | 53.8 s, 8993 MiB | 379.3 s, 8992 MiB |
| csv-direct | – | – | – | 52.6 s, 2150 MiB | – | – | 45.8 s, 6721 MiB | 351.2 s, 6721 MiB |

## 10 GB, master

Default limits:

| input | filter | aggregate | group_by | order_by | join | tumbling | sliding_count | sliding_time |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| jsonl-open | C 43.4 s, 3878 MiB | M 23.2 s, 3877 MiB | C 44.2 s, 3877 MiB | M 22.9 s, 3878 MiB | M 22.8 s, 3879 MiB | C 41.3 s, 3877 MiB | M 23.1 s, 3878 MiB | M 22.9 s, 3878 MiB |
| jsonl-documents | C 42.3 s, 3877 MiB | M 23.0 s, 3878 MiB | C 42.0 s, 3877 MiB | C 44.1 s, 3877 MiB | M 22.9 s, 3878 MiB | C 41.7 s, 3877 MiB | M 23.1 s, 3878 MiB | M 23.0 s, 3878 MiB |
| jsonl-direct | T 30.0 s, 38 MiB | T 30.0 s, 38 MiB | T 31.0 s, 327 MiB | T 31.2 s, 1385 MiB | T 30.0 s, 38 MiB | T 30.0 s, 39 MiB | M 23.0 s, 3877 MiB | M 23.1 s, 3877 MiB |
| jsonl-lines | T 30.0 s, 38 MiB | T 30.0 s, 38 MiB | T 30.8 s, 276 MiB | T 31.4 s, 1084 MiB | T 30.0 s, 38 MiB | T 30.0 s, 39 MiB | C 34.4 s, 3882 MiB | M 17.5 s, 3883 MiB |
| array-open | C 45.5 s, 3876 MiB | C 41.9 s, 3876 MiB | M 23.9 s, 3877 MiB | M 22.9 s, 3877 MiB | M 24.2 s, 3877 MiB | C 41.6 s, 3876 MiB | M 22.9 s, 3877 MiB | M 22.8 s, 3877 MiB |
| array-documents | C 41.2 s, 3876 MiB | C 41.7 s, 3876 MiB | M 23.0 s, 3877 MiB | C 41.4 s, 3876 MiB | C 41.6 s, 3877 MiB | C 41.3 s, 3876 MiB | M 22.9 s, 3877 MiB | C 41.1 s, 3876 MiB |
| array-direct | M 23.0 s, 3876 MiB | C 43.3 s, 3875 MiB | M 23.0 s, 3876 MiB | M 23.2 s, 3876 MiB | M 22.8 s, 3877 MiB | C 41.6 s, 3876 MiB | C 41.5 s, 3875 MiB | M 23.0 s, 3877 MiB |
| dir-documents | T 30.1 s, 126 MiB | T 30.1 s, 126 MiB | T 30.5 s, 268 MiB | T 31.2 s, 808 MiB | T 30.1 s, 126 MiB | T 30.1 s, 126 MiB | T 31.6 s, 2738 MiB | T 31.6 s, 2799 MiB |
| dir-direct | M 24.3 s, 3932 MiB | M 24.6 s, 3932 MiB | C 44.6 s, 3930 MiB | M 25.3 s, 3931 MiB | C 45.5 s, 3931 MiB | C 44.3 s, 3930 MiB | M 24.6 s, 3932 MiB | M 24.0 s, 3932 MiB |
| dir-lines | T 30.0 s, 114 MiB | T 30.0 s, 115 MiB | T 30.8 s, 347 MiB | T 31.3 s, 1119 MiB | T 30.0 s, 115 MiB | T 30.0 s, 115 MiB | M 18.0 s, 3928 MiB | C 36.5 s, 3926 MiB |
| csv-open | C 34.7 s, 3880 MiB | C 35.2 s, 3880 MiB | C 34.7 s, 3880 MiB | C 35.2 s, 3880 MiB | C 34.9 s, 3880 MiB | C 35.2 s, 3880 MiB | C 34.9 s, 3880 MiB | C 34.8 s, 3880 MiB |
| csv-documents | C 34.8 s, 3881 MiB | C 16.8 s, 3880 MiB | C 34.7 s, 3881 MiB | C 34.9 s, 3880 MiB | C 34.9 s, 3881 MiB | C 34.7 s, 3880 MiB | C 34.8 s, 3880 MiB | C 34.7 s, 3880 MiB |
| csv-direct | T 30.0 s, 37 MiB | T 30.0 s, 37 MiB | T 31.7 s, 565 MiB | T 31.4 s, 2341 MiB | T 30.0 s, 37 MiB | T 30.0 s, 37 MiB | C 32.7 s, 3879 MiB | M 15.5 s, 3880 MiB |

Extended limits:

| input | filter | aggregate | group_by | order_by | join | tumbling | sliding_count | sliding_time |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| jsonl-open | C 162.4 s, 14085 MiB | M 89.9 s, 14086 MiB | C 163.1 s, 14084 MiB | NR | NR | NR | NR | NR |
| jsonl-documents | C 163.5 s, 14085 MiB | C 162.3 s, 14085 MiB | C 163.3 s, 14085 MiB | NR | NR | NR | NR | NR |
| jsonl-direct | 462.0 s, 35 MiB | 448.2 s, 36 MiB | 509.9 s, 4603 MiB | NR | NR | NR | NR | NR |
| jsonl-lines | 606.4 s, 36 MiB | 587.1 s, 35 MiB | NR | NR | NR | NR | NR | NR |
| array-open | M 84.1 s, 14085 MiB | M 91.2 s, 14080 MiB | M 84.0 s, 14085 MiB | C 162.7 s, 14085 MiB | C 161.7 s, 14085 MiB | C 162.3 s, 14084 MiB | M 85.8 s, 14085 MiB | C 162.1 s, 14085 MiB |
| array-documents | M 83.6 s, 14085 MiB | M 84.1 s, 14086 MiB | M 83.8 s, 14085 MiB | M 85.8 s, 14085 MiB | C 162.4 s, 14085 MiB | M 85.9 s, 14085 MiB | M 84.2 s, 14085 MiB | C 163.7 s, 14084 MiB |
| array-direct | C 164.4 s, 14084 MiB | M 84.9 s, 14085 MiB | C 162.4 s, 14084 MiB | M 84.4 s, 14085 MiB | C 162.8 s, 14084 MiB | C 162.2 s, 14084 MiB | M 84.6 s, 14085 MiB | M 84.3 s, 14085 MiB |
| dir-documents | 789.3 s, 124 MiB | 784.7 s, 124 MiB | 706.1 s, 4949 MiB | NR | NR | NR | NR | NR |
| dir-direct | M 91.0 s, 14170 MiB | C 189.7 s, 14169 MiB | NR | NR | NR | NR | NR | NR |
| dir-lines | 614.8 s, 113 MiB | 606.4 s, 112 MiB | NR | NR | NR | NR | NR | NR |
| csv-open | C 140.1 s, 14099 MiB | C 140.0 s, 14098 MiB | C 141.6 s, 14099 MiB | C 139.5 s, 14098 MiB | C 62.6 s, 14098 MiB | NR | NR | NR |
| csv-documents | C 139.9 s, 14098 MiB | C 140.4 s, 14098 MiB | C 141.0 s, 14098 MiB | C 140.3 s, 14098 MiB | C 140.0 s, 14100 MiB | NR | NR | NR |
| csv-direct | 251.7 s, 35 MiB | 247.4 s, 35 MiB | 299.7 s, 4600 MiB | M 181.2 s, 13078 MiB | 413.3 s, 36 MiB | NR | NR | NR |

## 100 MB, `window-performance`

Default limits:

| input | tumbling | sliding_count | sliding_time |
|---|---:|---:|---:|
| jsonl-open | 8.0 s, 1399 MiB | 10.8 s, 1399 MiB | T 31.0 s, 1399 MiB |
| jsonl-documents | 8.0 s, 1399 MiB | 10.8 s, 1400 MiB | T 31.0 s, 1399 MiB |
| jsonl-direct | 5.7 s, 37 MiB | 8.3 s, 37 MiB | T 30.0 s, 39 MiB |
| jsonl-lines | 6.9 s, 38 MiB | 9.9 s, 37 MiB | T 30.0 s, 39 MiB |
| array-open | 7.2 s, 1114 MiB | 10.0 s, 1114 MiB | T 31.1 s, 1115 MiB |
| array-documents | 7.2 s, 1114 MiB | 10.0 s, 1114 MiB | T 31.1 s, 1115 MiB |
| array-direct | 6.0 s, 827 MiB | 8.8 s, 826 MiB | T 30.6 s, 828 MiB |
| dir-documents | 6.9 s, 40 MiB | 9.8 s, 40 MiB | T 30.0 s, 41 MiB |
| dir-direct | 6.0 s, 816 MiB | 8.8 s, 816 MiB | T 30.2 s, 818 MiB |
| dir-lines | 6.9 s, 38 MiB | 9.7 s, 38 MiB | T 30.0 s, 40 MiB |
| csv-open | 5.0 s, 944 MiB | 7.8 s, 944 MiB | T 30.3 s, 944 MiB |
| csv-documents | 5.0 s, 944 MiB | 7.9 s, 944 MiB | T 30.3 s, 944 MiB |
| csv-direct | 3.6 s, 38 MiB | 6.4 s, 37 MiB | T 30.0 s, 39 MiB |

Extended limits:

| input | tumbling | sliding_count | sliding_time |
|---|---:|---:|---:|
| jsonl-open | – | – | 69.7 s, 1399 MiB |
| jsonl-documents | – | – | 68.5 s, 1399 MiB |
| jsonl-direct | – | – | 68.4 s, 37 MiB |
| jsonl-lines | – | – | 71.2 s, 37 MiB |
| array-open | – | – | 68.5 s, 1113 MiB |
| array-documents | – | – | 67.6 s, 1114 MiB |
| array-direct | – | – | 69.0 s, 826 MiB |
| dir-documents | – | – | 69.7 s, 40 MiB |
| dir-direct | – | – | 68.0 s, 816 MiB |
| dir-lines | – | – | 71.2 s, 38 MiB |
| csv-open | – | – | 66.8 s, 944 MiB |
| csv-documents | – | – | 66.4 s, 944 MiB |
| csv-direct | – | – | 64.8 s, 37 MiB |

## 1 GB, `window-performance`

Default limits:

| input | tumbling | sliding_count | sliding_time |
|---|---:|---:|---:|
| jsonl-open | M 18.7 s, 3879 MiB | C 39.1 s, 3878 MiB | C 39.2 s, 3878 MiB |
| jsonl-documents | C 39.7 s, 3878 MiB | M 18.4 s, 3879 MiB | C 39.9 s, 3878 MiB |
| jsonl-direct | T 30.0 s, 40 MiB | T 30.0 s, 39 MiB | T 30.0 s, 39 MiB |
| jsonl-lines | T 30.0 s, 39 MiB | T 30.0 s, 39 MiB | T 30.0 s, 39 MiB |
| array-open | M 18.2 s, 3879 MiB | M 18.1 s, 3879 MiB | C 39.3 s, 3878 MiB |
| array-documents | C 44.3 s, 3877 MiB | C 39.1 s, 3878 MiB | C 39.2 s, 3877 MiB |
| array-direct | C 38.9 s, 3877 MiB | M 18.3 s, 3878 MiB | C 38.8 s, 3877 MiB |
| dir-documents | T 30.0 s, 53 MiB | T 30.0 s, 54 MiB | T 30.0 s, 53 MiB |
| dir-direct | C 37.7 s, 3892 MiB | C 39.5 s, 3892 MiB | M 18.7 s, 3894 MiB |
| dir-lines | T 30.0 s, 50 MiB | T 30.0 s, 50 MiB | T 30.0 s, 50 MiB |
| csv-open | C 32.0 s, 3881 MiB | C 33.3 s, 3882 MiB | C 34.1 s, 3882 MiB |
| csv-documents | C 34.3 s, 3882 MiB | C 12.7 s, 3881 MiB | C 12.7 s, 3881 MiB |
| csv-direct | 28.2 s, 38 MiB | T 30.0 s, 39 MiB | T 30.0 s, 39 MiB |

Extended limits:

| input | tumbling | sliding_count | sliding_time |
|---|---:|---:|---:|
| jsonl-open | 69.4 s, 13516 MiB | 91.0 s, 13516 MiB | 505.3 s, 13516 MiB |
| jsonl-documents | 69.3 s, 13516 MiB | 91.2 s, 13515 MiB | 523.2 s, 13515 MiB |
| jsonl-direct | 42.8 s, 36 MiB | 63.7 s, 36 MiB | 505.2 s, 36 MiB |
| jsonl-lines | 51.3 s, 37 MiB | 75.6 s, 37 MiB | 504.5 s, 37 MiB |
| array-open | 60.9 s, 10668 MiB | 83.1 s, 10668 MiB | 496.2 s, 10668 MiB |
| array-documents | 61.4 s, 10667 MiB | 82.5 s, 10668 MiB | 497.2 s, 10667 MiB |
| array-direct | 49.3 s, 7819 MiB | 71.8 s, 7819 MiB | 483.8 s, 7819 MiB |
| dir-documents | 54.2 s, 50 MiB | 78.5 s, 50 MiB | 521.0 s, 50 MiB |
| dir-direct | 48.2 s, 7785 MiB | 70.4 s, 7786 MiB | 486.1 s, 7786 MiB |
| dir-lines | 50.3 s, 47 MiB | 74.9 s, 47 MiB | 504.5 s, 47 MiB |
| csv-open | 40.9 s, 8992 MiB | 63.5 s, 8992 MiB | 475.7 s, 8992 MiB |
| csv-documents | 41.0 s, 8993 MiB | 63.5 s, 8992 MiB | 478.3 s, 8992 MiB |
| csv-direct | – | 48.7 s, 36 MiB | 489.5 s, 36 MiB |

## 10 GB, `window-performance`

Default limits:

| input | tumbling | sliding_count | sliding_time |
|---|---:|---:|---:|
| jsonl-open | C 41.8 s, 3875 MiB | M 14.9 s, 3878 MiB | C 31.4 s, 3877 MiB |
| jsonl-documents | C 34.1 s, 3877 MiB | C 33.1 s, 3877 MiB | M 15.1 s, 3878 MiB |
| jsonl-direct | T 30.0 s, 38 MiB | T 30.0 s, 38 MiB | T 30.0 s, 38 MiB |
| jsonl-lines | T 30.0 s, 38 MiB | T 30.0 s, 38 MiB | T 30.0 s, 38 MiB |
| array-open | M 15.0 s, 3878 MiB | M 15.6 s, 3878 MiB | C 37.3 s, 3877 MiB |
| array-documents | C 37.5 s, 3877 MiB | C 37.4 s, 3877 MiB | C 32.6 s, 3877 MiB |
| array-direct | C 32.8 s, 3876 MiB | M 15.6 s, 3877 MiB | C 33.2 s, 3877 MiB |
| dir-documents | T 30.0 s, 127 MiB | T 30.0 s, 127 MiB | T 30.0 s, 127 MiB |
| dir-direct | C 37.5 s, 3931 MiB | M 15.9 s, 3932 MiB | C 36.0 s, 3931 MiB |
| dir-lines | T 30.0 s, 116 MiB | T 30.0 s, 115 MiB | T 30.0 s, 116 MiB |
| csv-open | C 25.7 s, 3882 MiB | C 28.4 s, 3881 MiB | C 28.2 s, 3882 MiB |
| csv-documents | C 28.5 s, 3881 MiB | C 9.9 s, 3880 MiB | C 28.7 s, 3882 MiB |
| csv-direct | T 30.0 s, 38 MiB | T 30.0 s, 38 MiB | T 30.0 s, 38 MiB |

Extended limits:

| input | tumbling | sliding_count | sliding_time |
|---|---:|---:|---:|
| jsonl-open | C 150.1 s, 14086 MiB | M 59.2 s, 14081 MiB | C 145.9 s, 14085 MiB |
| jsonl-documents | C 144.2 s, 14086 MiB | C 150.9 s, 14085 MiB | M 60.7 s, 14086 MiB |
| jsonl-direct | 335.9 s, 36 MiB | 515.4 s, 36 MiB | T 1200.0 s, 37 MiB |
| jsonl-lines | 404.9 s, 36 MiB | 578.7 s, 36 MiB | T 1200.0 s, 38 MiB |
| array-open | M 54.9 s, 14086 MiB | M 55.1 s, 14086 MiB | C 143.3 s, 14085 MiB |
| array-documents | M 56.0 s, 14086 MiB | M 55.1 s, 14086 MiB | M 56.7 s, 14086 MiB |
| array-direct | M 55.6 s, 14086 MiB | M 55.3 s, 14086 MiB | M 54.6 s, 14085 MiB |
| dir-documents | 443.2 s, 124 MiB | 632.2 s, 125 MiB | T 1200.0 s, 126 MiB |
| dir-direct | M 56.5 s, 14172 MiB | C 145.4 s, 14170 MiB | M 58.0 s, 14167 MiB |
| dir-lines | 426.9 s, 114 MiB | 595.7 s, 113 MiB | T 1200.0 s, 114 MiB |
| csv-open | C 129.6 s, 14099 MiB | C 127.8 s, 14098 MiB | C 128.9 s, 14099 MiB |
| csv-documents | C 36.4 s, 14097 MiB | C 128.9 s, 14098 MiB | C 38.1 s, 14098 MiB |
| csv-direct | 212.9 s, 35 MiB | 381.8 s, 35 MiB | T 1200.0 s, 36 MiB |

## `dir:open` on JSON Lines

`bench/scale/diag/`: the sum of `seq` over each expression, master. `F` is `data.jsonl`.
The expression `parse` is the one in `dir:parse`:
`let $items := parse-json(file:read-text(F)) return if (count($items) eq 1) then $items else [ $items ]`.

| expression | 100 MB, default | 1 GB, default | 1 GB, extended | 10 GB, default | 10 GB, extended |
|---|---:|---:|---:|---:|---:|
| `parse-json(file:read-text(F))` | 4.3 s, 37 MiB | 26.6 s, 37 MiB | 26.6 s, 37 MiB | T 30.0 s, 38 MiB | 431.2 s, 36 MiB |
| `try { parse-json(...) } catch * { () }` | 4.9 s, 813 MiB | M 15.6 s, 3878 MiB | 34.1 s, 7755 MiB | M 23.0 s, 3876 MiB | C 170.8 s, 14085 MiB |
| `let $items := parse-json(...)`, then `count($items)` and `$items` | 5.0 s, 813 MiB | M 15.5 s, 3878 MiB | 34.6 s, 7755 MiB | M 23.3 s, 3877 MiB | C 163.0 s, 14084 MiB |
| `[ parse-json(...) ][]` | 5.7 s, 825 MiB | C 33.3 s, 3877 MiB | 40.0 s, 7819 MiB | C 44.9 s, 3876 MiB | C 173.7 s, 14085 MiB |
| `parse`, then `[]` | 6.2 s, 1107 MiB | C 31.9 s, 3877 MiB | 45.5 s, 10618 MiB | C 42.3 s, 3875 MiB | C 164.3 s, 14084 MiB |
| `try { { "content" : parse } } catch * { ... }`, then `.content[]` | 7.1 s, 1398 MiB | C 31.3 s, 3877 MiB | 56.6 s, 13515 MiB | C 41.7 s, 3875 MiB | C 165.6 s, 14084 MiB |
| `dir:open($dir:root, "data.jsonl").content[]` | 7.1 s, 1399 MiB | C 31.2 s, 3877 MiB | 54.1 s, 13516 MiB | C 41.5 s, 3876 MiB | M 84.3 s, 14086 MiB |
| `dir:documents($dir:root).content[]` | 7.1 s, 1399 MiB | M 15.8 s, 3879 MiB | 55.2 s, 13516 MiB | C 41.9 s, 3876 MiB | M 84.1 s, 14086 MiB |

- `dir:open` on JSON Lines holds the whole file: 13516 MiB at 1 GB, 14.2 times the file,
  2205 bytes per record of 156 bytes.
- `try` around a lazy `parse-json` holds the items: 37 MiB without it, 813 MiB with it.
- So does `count` on the `let` variable, and so does the array constructor, each with
  the same order of memory.
- They add up: `parse` without `try` takes 1107 MiB at 100 MB, inside the `try` of
  `dir:read` 1398 MiB, the same as `dir:open`.

At 100 MB and 1 GB, `dir:open` and `dir:documents` take the same memory on `array` (11
times the file at 1 GB, 10668 MiB) and on `csv` (21.6 times the file, 8992 MiB). On `dir`,
`dir:documents` holds one file at a time: 39 MiB at 100 MB and 124 MiB at 10 GB for
`filter`.

Not investigated: why `dir-direct`, a `for` with `order by` that returns `parse-json`,
takes 816 MiB at 100 MB, when `dir-lines`, of the same shape, takes 37 MiB.

## What is missing

- 47 cells of master at 10 GB with the extended limits did not run, marked `NR` in its
  table: the budget ran out. On the paths that hold the input at 10 GB the other cells
  stopped at the memory limit. On `jsonl-direct`, `jsonl-lines`, `dir-documents` and
  `dir-lines` there is no 10 GB result on master for `order_by`, `join` and the window
  operations, on `jsonl-lines` and `dir-lines` none for `group_by`, and on `csv-direct`
  none for the window operations. All cells ran with the default limits.
- `window-performance` ran the window operations only.
- `sliding_time` at 10 GB on `window-performance` reached the 1200 s limit on every path
  that did not run out of memory first.
- At 1 GB and 10 GB each cell is one run.

## Running out of memory: before and after PR #178

1 GB, default limits (`--max-memory 4294967296`), one run per cell, inputs `csv-open`,
`jsonl-open`, `array-open`, 8 operations each. Peak RSS was about 3.9 GB in all cells.

| run | engine | `memory_limit`, server answers | no response, session closed |
|---|---|---:|---:|
| 41659837513 | `7c8d649f97` | 6 | 18 |
| 41728338941 | `669ba8cf81`, in PR #178 | 24 | 0 |

The cells that reached the limit took 12 to 17 s in the second run. In the first, the
cells whose session closed took 27 to 39 s, and ended with `dumped core`.
