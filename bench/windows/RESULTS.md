# Window clauses over large irregular series

Measured on GitHub Actions `ubuntu-24.04` runners (16 GB), Release build
(`full-release`, Arch builder image), 2026-11-03: run 41614515910 before the change
(the code of `8b3cae940`) and run 41614517411 after it (the code of `d67c6a0ff`), one
runner each, in parallel. Each cell is a fresh process: the median of 3 runs below 10^6
records, one run at 10^6. Peak RSS comes from `wait4`. Run with `bench/windows/run.sh`
(see its header); `gen.jq` writes the files and `lib.xqm` holds the record shape. Every
query gave the same result before and after, on all inputs and sizes measured in both.

The records are irregular: about 1 in 7 has no `v`, 1 in 11 a null `v`, 1 in 13 a
string `v`, 1 in 3 a nested `meta`, 1 in 17 an `extra` array. One second separates
consecutive records, ten minutes every 100 records. The JSON Lines file has 77 MB at
10^6 records. The queries end in one number, so that results take no memory.

Inputs:

- `gen`: `parse-json` of a string made per record while the query runs.
- `lines`: `file:read-text-lines` of a JSON Lines file, `parse-json` per line.
- `whole`: `parse-json(file:read-text(...))` over the JSON Lines file.
- `array`: `parse-json(file:read-text(...))[]` over one JSON array.

## Findings

- **Sliding windows hold their open windows, not the input.** A moving average over 10
  points takes 36 MiB at 10^6 records from `gen`, `lines` or `whole`, against 1157, 1157
  and 705 MiB before. `array` stays at 721 MiB: the parser holds the input before any
  window sees it.
- **Windows that stay open cost linear time when the end condition reads no start
  variable:** 10^6 records take 14 s. The same query took 28.7 s at 16000 records.
- **Items that a window can still return stay in memory:** with `$w` read by the return
  clause, a window that never closes keeps 1182 MiB at 10^6 records, since any later
  record could close it. Without a reference to `$w` or to the items of the start
  and end variables, 58 MiB, of which about 16 MiB are the positions of 10^6 windows.
- **Two forms stay quadratic:** see [Windows that stay open](#windows-that-stay-open).
- **One form got slower:** `never_closes_start`, 16000 records, 45.71 s to 67.10 s;
  8000 records, 10.94 s to 14.19 s. Below 8000 records it is equal or faster.

## Before and after, 10^6 records

Seconds and peak RSS in MiB.

| query | input | before s | after s | before MiB | after MiB |
|---|---|---:|---:|---:|---:|
| baseline | gen | 15.36 | 14.20 | 36 | 36 |
| baseline | lines | 5.07 | 4.73 | 36 | 35 |
| baseline | whole | 3.73 | 3.77 | 36 | 36 |
| baseline | array | 4.45 | 4.25 | 720 | 720 |
| moving_points | gen | 21.18 | 20.57 | 1157 | 36 |
| moving_points | lines | 11.32 | 10.68 | 1157 | 35 |
| moving_points | whole | 9.80 | 9.29 | 705 | 36 |
| moving_points | array | 9.85 | 9.93 | 721 | 721 |
| moving_minutes | gen | 84.92 | 76.11 | 1158 | 36 |
| moving_minutes | lines | 75.36 | 64.65 | 1157 | 36 |
| moving_minutes | whole | 73.21 | 66.17 | 705 | 36 |
| moving_minutes | array | 73.39 | 62.92 | 721 | 721 |
| duration | gen | 17.29 | 15.94 | 35 | 36 |
| duration | lines | 6.76 | 6.51 | 36 | 36 |
| duration | whole | 5.45 | 5.29 | 36 | 36 |
| duration | array | 5.74 | 5.57 | 721 | 721 |
| sessions | gen | 16.81 | 15.46 | 35 | 35 |
| sessions | lines | 6.49 | 6.03 | 36 | 35 |
| sessions | whole | 5.05 | 4.85 | 35 | 36 |
| sessions | array | 5.34 | 5.12 | 720 | 721 |

- `baseline`: no window, the input alone.
- `moving_points`: sliding, average over 10 points.
- `moving_minutes`: sliding, average over 1 minute from each record (about 60 points).
- `duration`: tumbling, windows of 1 minute from their first record.
- `sessions`: tumbling, a new window after a gap above 5 minutes.

## Time and peak memory after the change

Seconds and MiB per query and input.

| query | records | gen s | gen MiB | lines s | lines MiB | whole s | whole MiB | array s | array MiB |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| baseline | 10000 | 0.17 | 36 | 0.08 | 36 | 0.06 | 36 | 0.07 | 41 |
| moving_points | 10000 | 0.23 | 36 | 0.13 | 36 | 0.12 | 36 | 0.12 | 42 |
| moving_minutes | 10000 | 0.78 | 36 | 0.67 | 36 | 0.68 | 36 | 0.68 | 42 |
| duration | 10000 | 0.18 | 36 | 0.09 | 36 | 0.08 | 36 | 0.08 | 42 |
| sessions | 10000 | 0.22 | 36 | 0.11 | 36 | 0.07 | 36 | 0.08 | 41 |
| baseline | 100000 | 1.42 | 36 | 0.51 | 36 | 0.39 | 36 | 0.45 | 103 |
| moving_points | 100000 | 2.02 | 36 | 1.11 | 36 | 0.96 | 36 | 1.02 | 103 |
| moving_minutes | 100000 | 7.40 | 36 | 6.45 | 36 | 6.31 | 36 | 6.34 | 103 |
| duration | 100000 | 1.63 | 36 | 0.68 | 36 | 0.55 | 36 | 0.58 | 103 |
| sessions | 100000 | 1.54 | 36 | 0.63 | 36 | 0.51 | 36 | 0.54 | 103 |
| baseline | 1000000 | 14.20 | 36 | 4.73 | 35 | 3.77 | 36 | 4.25 | 720 |
| moving_points | 1000000 | 20.57 | 36 | 10.68 | 35 | 9.29 | 36 | 9.93 | 721 |
| moving_minutes | 1000000 | 76.11 | 36 | 64.65 | 36 | 66.17 | 36 | 62.92 | 721 |
| duration | 1000000 | 15.94 | 36 | 6.51 | 36 | 5.29 | 36 | 5.57 | 721 |
| sessions | 1000000 | 15.46 | 35 | 6.03 | 35 | 4.85 | 36 | 5.12 | 721 |

## Memory against the window size

`tumbling_points`, input `gen`, after the change: consecutive blocks of k records.

| records | k | seconds | peak RSS MiB |
|---:|---:|---:|---:|
| 10000 | 10 | 0.17 | 36 |
| 10000 | 1000 | 0.16 | 36 |
| 100000 | 10 | 1.49 | 36 |
| 100000 | 1000 | 1.42 | 36 |
| 100000 | 100000 | 1.49 | 147 |
| 1000000 | 10 | 14.70 | 35 |
| 1000000 | 1000 | 14.10 | 36 |
| 1000000 | 100000 | 14.06 | 147 |

## Windows that stay open

Input `gen`; `only` means `only end`, so no window is returned. `$w` read means that the
return clause refers to the window variable.

| query | form | growth after the change |
|---|---|---|
| `never_closes` | sliding, `only`, end on the current item, `$w` not read | linear time, memory for positions |
| `never_closes_read` | the same, `$w` read in a clause that never runs | linear time, memory holds the input |
| `never_closes_start` | sliding, `only`, end reads the start item | **quadratic time** |
| `open_at_end` | sliding, no `only`, end on the current item, `$w` not read | linear time, memory for positions |
| `open_at_end_read` | the same, every `$w` read | **quadratic time**: n(n+1)/2 items read |
| `first_stays_open` | the first window never closes, the others close and wait | linear in these sizes |
| `tumbling_never` | one tumbling window that never closes, `$w` read | linear, before and after; memory holds the input |

| query | records | before s | after s | before MiB | after MiB |
|---|---:|---:|---:|---:|---:|
| never_closes | 1000 | 0.15 | 0.04 | 36 | 36 |
| never_closes | 2000 | 0.49 | 0.06 | 37 | 36 |
| never_closes | 4000 | 1.84 | 0.08 | 39 | 36 |
| never_closes | 8000 | 7.22 | 0.14 | 44 | 36 |
| never_closes | 16000 | 28.66 | 0.27 | 53 | 36 |
| never_closes | 100000 | - | 1.43 | - | 37 |
| never_closes | 1000000 | - | 14.00 | - | 58 |
| never_closes_read | 1000 | 0.15 | 0.04 | 36 | 36 |
| never_closes_read | 2000 | 0.50 | 0.06 | 37 | 37 |
| never_closes_read | 4000 | 1.85 | 0.08 | 39 | 39 |
| never_closes_read | 8000 | 7.23 | 0.14 | 44 | 44 |
| never_closes_read | 16000 | 28.49 | 0.26 | 53 | 53 |
| never_closes_read | 100000 | - | 1.48 | - | 149 |
| never_closes_read | 1000000 | - | 14.72 | - | 1182 |
| never_closes_start | 1000 | 0.21 | 0.19 | 36 | 36 |
| never_closes_start | 2000 | 0.72 | 0.65 | 37 | 37 |
| never_closes_start | 4000 | 2.74 | 2.73 | 39 | 39 |
| never_closes_start | 8000 | 10.94 | 14.19 | 44 | 44 |
| never_closes_start | 16000 | 45.71 | 67.10 | 53 | 53 |
| open_at_end | 1000 | 0.15 | 0.04 | 36 | 36 |
| open_at_end | 2000 | 0.50 | 0.05 | 37 | 36 |
| open_at_end | 4000 | 1.86 | 0.08 | 39 | 36 |
| open_at_end | 8000 | 7.18 | 0.14 | 44 | 36 |
| open_at_end | 16000 | 28.54 | 0.27 | 53 | 36 |
| open_at_end | 100000 | - | 1.43 | - | 37 |
| open_at_end | 1000000 | - | 14.18 | - | 58 |
| open_at_end_read | 1000 | 0.16 | 0.05 | 36 | 36 |
| open_at_end_read | 2000 | 0.52 | 0.09 | 37 | 37 |
| open_at_end_read | 4000 | 1.93 | 0.22 | 39 | 39 |
| open_at_end_read | 8000 | 7.51 | 0.71 | 44 | 44 |
| open_at_end_read | 16000 | 30.28 | 3.16 | 53 | 53 |
| first_stays_open | 1000 | 0.05 | 0.05 | 36 | 36 |
| first_stays_open | 2000 | 0.06 | 0.06 | 37 | 36 |
| first_stays_open | 4000 | 0.10 | 0.09 | 39 | 36 |
| first_stays_open | 8000 | 0.18 | 0.16 | 44 | 36 |
| first_stays_open | 16000 | 0.39 | 0.28 | 53 | 36 |
| tumbling_never | 1000 | 0.04 | 0.04 | 36 | 36 |
| tumbling_never | 2000 | 0.06 | 0.05 | 37 | 37 |
| tumbling_never | 4000 | 0.09 | 0.09 | 39 | 39 |
| tumbling_never | 8000 | 0.15 | 0.14 | 44 | 44 |
| tumbling_never | 16000 | 0.28 | 0.26 | 53 | 53 |
| tumbling_never | 100000 | - | 1.48 | - | 147 |
| tumbling_never | 1000000 | - | 14.59 | - | 1158 |

The two quadratic forms cannot be avoided by this change:

- `never_closes_start`: each window has its own value of the condition, n windows are
  open by the specification, and nothing in the condition proves that a window can no
  longer close, so n(n+1)/2 evaluations remain. Before, it took 45.71 s at 16000
  records; after, 67.10 s.
- `open_at_end_read`: n windows are returned by the specification and the query reads
  each of them: the sum of their sizes is 128008000 at 16000 records.

## Why, in the code

`src/runtime/core/gflwor/window_iterator.cpp`, `nextImpl`, buffers the binding sequence
in a temp sequence (`theDomainSeq`), and a window is a pair of positions in it.

- The temp sequence is lazy (`SimpleLazyTempSeq`), for sliding clauses as for tumbling
  ones, unless the FLWOR or a condition of a sliding clause has side effects
  (`window_has_side_effects` in `plan_visitor.cpp`), so that reading the input does not
  interleave with them.
- After each returned window and after each item, `doGarbageCollection` purges up to
  the item before the start of the first open window, which `previous` may read, or up
  to the item before the current one if no window is open. It runs after the consumer
  has asked for the next tuple: `order by`, `group by` and function items copy the
  window by then.
- If nothing refers to the window variable, to the start variables or to the items of
  the end variables after the clause, no window needs items, and only the current item
  and its neighbors stay.
- An end condition that reads no start variable, and has only built-in deterministic
  functions and no constructor, is evaluated once per item instead of once per open window
  (`same_end_cond_for_all_windows`). Other conditions are still evaluated per window.
- A domain that does not depend on an enclosing `for` can be hoisted out of it by the
  optimizer: a `let` then holds the input for all the iterations.

## Optimization candidates

None of these is implemented.

| candidate | expected effect |
|---|---|
| Hash index of the open windows on an equality between a start and an end variable | Time of a session-like condition `$s.id eq $e.id` from records times open windows to records times matches. The evaluations it skips must not be able to raise an error. |
| Stop testing an open window once its condition cannot change (for example `$e - $s eq k` past k) | Avoids the quadratic cost only for conditions the compiler can prove monotonic. Only for `only end`: without it, the window is returned at the end. |
| Keep in each window only the items its start and end variables read, when `$w` has no reference | Queries on `$s` and `$e` alone with many open windows, from the input to the items read. |
| Incremental `count`, `sum` and `avg` on sliding windows | `moving_minutes` from records times window size to records. Only for these aggregates over `$w` or a navigation on it. |
| Streaming `[]` on a top-level array in a file | `array` input at the 36 MiB of `whole`; `jsoniq-strip-top-level-array` is the existing path to measure first. |
