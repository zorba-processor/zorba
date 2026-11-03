# Semi-structured time series through zorba-mcp

Measured on GitHub Actions `ubuntu-24.04` runners, Release build (`full-release`,
Arch builder image), 2026-11-03. Every cell goes through `run_query` of `zorba-mcp`.

| engine | commit | run | jobs: size, CPU |
|---|---|---|---|
| master | `7c8d649f97` | 41659837513 | 100 MB: EPYC 7763; 1 GB: EPYC 9V45; 10 GB: EPYC 7763 |
| `window-performance` (PR #169, in progress) | `eabbb66ea3` | 41659857371 | 100 MB: EPYC 7763; 1 GB: EPYC 9V74; 10 GB: EPYC 9V45 |

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

##  , master

Default limits:


Extended limits:


##  , master

Default limits:


Extended limits:


##  , master

Default limits:


Extended limits:


##  , `window-performance`

Default limits:


Extended limits:


##  , `window-performance`

Default limits:


Extended limits:


##  , `window-performance`

Default limits:


Extended limits:


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
  stopped at the memory limit; on `jsonl-direct`, `jsonl-lines`, `dir-documents`,
  `dir-lines` and `csv-direct` there is no 10 GB result on master for the window
  operations, nor for `order_by` and `join` except `csv-direct`. All cells ran with the
  default limits.
- `window-performance` ran the window operations only.
- `sliding_time` at 10 GB on `window-performance` reached the 1200 s limit on every path
  that did not run out of memory first.
- At 1 GB and 10 GB each cell is one run.
