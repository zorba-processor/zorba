# Window clauses over large irregular series

Measured on a GitHub Actions `ubuntu-24.04` runner (16 GB), Release build
(`full-release`, Arch builder image), 2026-11-02, commit `93e5a6dab`, run
41538186627. Each cell is a fresh process: the median of 3 runs below 10^6 records,
one run at 10^6. Peak RSS comes from `wait4`. Run with `bench/windows/run.sh`
(see its header); `gen.jq` writes the files and `lib.xqm` holds the record shape.

The records are irregular: about 1 in 7 has no `v`, 1 in 11 a null `v`, 1 in 13 a
string `v`, 1 in 3 a nested `meta`, 1 in 17 an `extra` array. One second separates
consecutive records, ten minutes every 100 records. The JSON Lines file has 77 MB at
10^6 records. The queries end in one number, so that results take no memory; each
query gave the same number on all four inputs.

Inputs:

- `gen`: `parse-json` of a string made per record while the query runs.
- `lines`: `file:read-text-lines` of a JSON Lines file, `parse-json` per line.
- `whole`: `parse-json(file:read-text(...))` over the JSON Lines file.
- `array`: `parse-json(file:read-text(...))[]` over one JSON array.

## Findings

- **Time is linear in the number of records** when the number of open windows is
  bounded, on every query and input below.
- **A tumbling window holds memory for its window, not for the input.** With `whole`
  or `lines`, 10^6 records take 36 MiB, as at 10^4. With windows of 100000 records,
  147 MiB at both 10^5 and 10^6 records.
- **A sliding window holds the whole input.** A moving average over 10 points takes
  1157 MiB at 10^6 records from `lines` and 705 MiB from `whole`, against 36 MiB
  without the window.
- **A JSON array parsed whole held the input before any window saw it:** 721 MiB at
  10^6 records, with or without a window clause, against 35 MiB for JSON Lines through
  `whole`. `parse-json(...)[]` takes 36 MiB with the option `zorba-streaming-members`;
  see "One large JSON array".
- **Sliding windows that stay open cost quadratic time.** Doubling the records takes
  4 times as long, because each record evaluates the end condition of every open window.

## Time and peak memory

Seconds and MiB per query and input.

| query | records | gen s | gen MiB | lines s | lines MiB | whole s | whole MiB | array s | array MiB |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| baseline | 10000 | 0.15 | 36 | 0.07 | 36 | 0.06 | 36 | 0.07 | 41 |
| moving_points | 10000 | 0.21 | 46 | 0.13 | 46 | 0.12 | 41 | 0.12 | 41 |
| moving_minutes | 10000 | 0.73 | 46 | 0.65 | 46 | 0.64 | 41 | 0.64 | 41 |
| duration | 10000 | 0.17 | 36 | 0.08 | 35 | 0.08 | 36 | 0.08 | 41 |
| sessions | 10000 | 0.16 | 36 | 0.08 | 35 | 0.07 | 36 | 0.07 | 41 |
| baseline | 100000 | 1.29 | 36 | 0.47 | 36 | 0.37 | 36 | 0.44 | 103 |
| moving_points | 100000 | 1.92 | 147 | 1.11 | 147 | 0.98 | 101 | 0.99 | 103 |
| moving_minutes | 100000 | 7.07 | 147 | 6.26 | 147 | 6.15 | 102 | 6.14 | 103 |
| duration | 100000 | 1.45 | 36 | 0.62 | 36 | 0.52 | 35 | 0.55 | 103 |
| sessions | 100000 | 1.40 | 36 | 0.58 | 35 | 0.48 | 36 | 0.51 | 103 |
| baseline | 1000000 | 12.80 | 35 | 4.57 | 36 | 3.43 | 35 | 4.20 | 721 |
| moving_points | 1000000 | 19.14 | 1158 | 10.76 | 1157 | 9.51 | 705 | 9.72 | 721 |
| moving_minutes | 1000000 | 71.04 | 1157 | 62.67 | 1157 | 61.57 | 705 | 61.30 | 721 |
| duration | 1000000 | 14.57 | 36 | 5.96 | 35 | 4.98 | 35 | 5.21 | 721 |
| sessions | 1000000 | 13.96 | 35 | 5.57 | 36 | 4.61 | 35 | 4.92 | 721 |

- `baseline`: no window, the input alone.
- `moving_points`: sliding, average over 10 points.
- `moving_minutes`: sliding, average over 1 minute from each record (about 60 points).
- `duration`: tumbling, windows of 1 minute from their first record.
- `sessions`: tumbling, a new window after a gap above 5 minutes.

## Memory against the window size

`tumbling_points`, input `gen`: consecutive blocks of k records.

| records | k | seconds | peak RSS MiB |
|---:|---:|---:|---:|
| 10000 | 10 | 0.16 | 36 |
| 10000 | 1000 | 0.15 | 36 |
| 100000 | 10 | 1.37 | 36 |
| 100000 | 1000 | 1.30 | 36 |
| 100000 | 100000 | 1.37 | 147 |
| 1000000 | 10 | 13.56 | 35 |
| 1000000 | 1000 | 12.82 | 36 |
| 1000000 | 100000 | 13.13 | 147 |

## Windows that stay open

`never_closes`, input `gen`: `sliding`, `start when true`, an `only end` condition
that no record meets.

| records | seconds | peak RSS MiB |
|---:|---:|---:|
| 1000 | 0.15 | 36 |
| 2000 | 0.50 | 37 |
| 4000 | 1.86 | 39 |
| 8000 | 7.24 | 44 |
| 16000 | 28.67 | 53 |

10^6 records would take about 31 hours at this rate (extrapolated, not measured).

## Why, in the code

`src/runtime/core/gflwor/window_iterator.cpp`, `nextImpl`, buffers the binding sequence
in a temp sequence (`theDomainSeq`).

- Sliding: `window_clause` is created with `lazy = false`
  (`create_window_clause` default; its constructor sets lazy only for tumbling), so
  `createTempSeq(iterator, false)` builds a `SimpleTempSeq`, which reads the whole
  binding sequence before the first window. It is never purged: no caller passes
  `maxNeededHistory`, and the call to `doGarbageCollection` in the sliding branch
  is commented out.
- Tumbling: the sequence is lazy (`SimpleLazyTempSeq`) and `theMaxNeededHistory` is 1.
  After each closed window `doGarbageCollection` purges everything but the last item,
  which a later `previous` may read. It holds the items since the last closed window:
  the largest window plus the largest run of items outside any window. A window that
  never closes keeps all the items after its start.
- Open windows: each record evaluates the end condition of every open window, in the
  loop over `theOpenWindows`, so a sliding clause costs records times open windows.
- Aggregates read `$w` from the temp sequence, per window: a moving average costs
  records times window size.

## One large JSON array

`run_array.sh`, run 41645367526: three runs in one job on an `ubuntu-24.04` runner, the
lowest time of 3 runs and the largest peak RSS. Before is `7c8d649f9`, after is
`9fb980aba`. 10^6 records, a 78 MB array file. `filter` is `filter.jq`, `aggregate` is
`baseline`, `tumbling` is `sessions`. The option is
`{ "zorba-streaming-members" : true }`.

| query | input | before ms | before MiB | after ms | after MiB |
|---|---|---:|---:|---:|---:|
| filter | `parse-json(file:read-text(F))[]` | 4346 | 722 | 4596 | 722 |
| aggregate | same | 4807 | 722 | 4524 | 722 |
| tumbling | same | 5454 | 722 | 5541 | 722 |
| filter | same, with the option | - | - | 6177 | 36 |
| aggregate | same, with the option | - | - | 6449 | 36 |
| tumbling | same, with the option | - | - | 7909 | 36 |
| filter | an ordinary string, with the option | - | - | 6046 | 365 |
| aggregate | an ordinary string, with the option | - | - | 6344 | 365 |
| tumbling | an ordinary string, with the option | - | - | 7674 | 365 |
| filter | `dir:open(D, F).content[]` | 5190 | 985 | 5409 | 985 |
| aggregate | same | 5426 | 985 | 5511 | 985 |
| tumbling | same | 6337 | 986 | 6875 | 986 |
| first member | `parse-json(file:read-text(F))[][1].id` | 3928 | 720 | 3954 | 720 |
| first member | same, with the option | - | - | 2477 | 36 |

With the option, a query that reads all the members takes 34% to 43% more time than the
same build without it, for the second parse of each top-level value; one that stops at
the first member takes 37% less. Without the option the plan is the one before; the time
differences above are of both signs, between -6% and +8%.

The first parse checks the JSON without building items. In the same job, with a first
parse that built the items and dropped them:

| query | with the option, building items: ms | not building them: ms |
|---|---:|---:|
| filter | 6819 | 6177 |
| aggregate | 7111 | 6449 |
| tumbling | 8385 | 7909 |
| first member | 3148 | 2477 |

Where the memory goes, as peak RSS without the option:

| query | ms | peak RSS MiB |
|---|---:|---:|
| `1` (the process) | 22 | 35 |
| `string-length(file:read-text(F))` (the text in memory) | 439 | 165 |
| `size(parse-json(file:read-text(F)))` | 3930 | 720 |
| `size(parse-json(file:read-text(N)))`, N holding 10^6 nulls | 173 | 53 |

Of the 685 MiB above the process, the array takes about 18 MiB and the members about 667
MiB, 700 bytes per record. `parse-json(file:read-text(F))` reads the text from the file,
so it holds none of it.

## Optimization candidates

None of these is implemented.

| candidate | expected effect |
|---|---|
| Lazy temp sequence for sliding windows, purging before the start of the first open window | Peak memory of `moving_points` and `moving_minutes` from `lines` or `whole` falls from about 1.2 or 0.7 KiB per record to the size of the open windows, as in tumbling. Needs `previous` kept and a check that `$w` has no reader once the tuple is consumed, which tumbling already relies on. |
| Stop testing an open window once its condition cannot change (for example `$e - $s eq k` past k) | Avoids the quadratic cost only for conditions the compiler can prove monotonic; the general case stays. |
| Incremental `count`, `sum` and `avg` on sliding windows | `moving_minutes` from records times window size to records. Only for these aggregates over `$w` or a navigation on it. |
| `std::deque` instead of `std::vector` in the lazy temp sequence and for `theOpenWindows` | Purging and closing the first window are linear in what remains. No slowdown appears at k = 100000, so the expected effect is small. |
