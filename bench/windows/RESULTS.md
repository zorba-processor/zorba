# Window clauses over large irregular series

Measured on one GitHub Actions `ubuntu-24.04` runner (16 GB), Release build
(`full-release`, Arch builder image), 2026-11-03, run 41635559360: first with the window
code of `8b3cae940` (before), then with the head of this change (after), in the same
job. Each cell is a fresh process: the median of 3 runs below 10^6 records, one run at
10^6. Peak RSS comes from `wait4`. Run with `bench/windows/run.sh` (see its header);
`gen.jq` writes the files and `lib.xqm` holds the record shape. Every query gave the same
result before and after, on all inputs and sizes measured in both.

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

- **Sliding windows hold their open windows, not the input,** in a query where no side
  effect can run. A moving average over 10 points takes 38-39 MiB at 10^6 records from
  `gen`, `lines` or `whole`, against 1160, 1160 and 708 MiB before. `array` stays at
  724 MiB: the parser holds the input before any window sees it.
- **Windows that stay open cost linear time when the end condition is shared:** 10^6
  records take 15 s. The same query took 28.7 s at 16000 records.
- **Items that a window can still return stay in memory:** with `$w` read by the return
  clause, a window that never closes keeps 1185 MiB at 10^6 records, since any later
  record could close it. Without a reference to `$w` or to the items of the start
  and end variables, 61 MiB, of which about 16 MiB are the positions of 10^6 windows.
- **Two forms stay quadratic:** see [Windows that stay open](#windows-that-stay-open).
- **What got slower:** `moving_minutes` at 10^6 records, by 4% to 7% on every input;
  `moving_points` from `gen`, by 5%. The other queries with a window are within about
  3%, except `duration` from `array` at 6%, where the input is parsed first.

## Before and after, 10^6 records

Seconds and peak RSS in MiB.

| query | input | before s | after s | before MiB | after MiB |
|---|---|---:|---:|---:|---:|
| baseline | gen | 14.73 | 14.96 | 38 | 39 |
| baseline | lines | 5.06 | 4.92 | 38 | 39 |
| baseline | whole | 3.65 | 3.63 | 38 | 39 |
| baseline | array | 4.41 | 4.39 | 723 | 723 |
| moving_points | gen | 21.08 | 22.12 | 1160 | 38 |
| moving_points | lines | 11.19 | 11.12 | 1160 | 39 |
| moving_points | whole | 9.80 | 9.62 | 708 | 38 |
| moving_points | array | 9.85 | 9.94 | 724 | 724 |
| moving_minutes | gen | 79.13 | 83.81 | 1160 | 39 |
| moving_minutes | lines | 69.17 | 74.01 | 1160 | 39 |
| moving_minutes | whole | 67.11 | 69.93 | 708 | 38 |
| moving_minutes | array | 67.61 | 70.95 | 724 | 724 |
| duration | gen | 16.63 | 16.87 | 38 | 38 |
| duration | lines | 6.61 | 6.70 | 38 | 39 |
| duration | whole | 5.44 | 5.37 | 38 | 38 |
| duration | array | 5.60 | 5.92 | 724 | 724 |
| sessions | gen | 16.30 | 16.31 | 38 | 38 |
| sessions | lines | 6.09 | 6.29 | 38 | 39 |
| sessions | whole | 5.02 | 4.99 | 38 | 39 |
| sessions | array | 5.21 | 5.37 | 724 | 724 |

- `baseline`: no window, the input alone.
- `moving_points`: sliding, average over 10 points.
- `moving_minutes`: sliding, average over 1 minute from each record (about 60 points).
- `duration`: tumbling, windows of 1 minute from their first record.
- `sessions`: tumbling, a new window after a gap above 5 minutes.

## Time and peak memory after the change

Seconds and MiB per query and input.

| query | records | gen s | gen MiB | lines s | lines MiB | whole s | whole MiB | array s | array MiB |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| baseline | 10000 | 0.17 | 39 | 0.07 | 39 | 0.06 | 39 | 0.06 | 44 |
| moving_points | 10000 | 0.24 | 39 | 0.13 | 38 | 0.12 | 39 | 0.12 | 44 |
| moving_minutes | 10000 | 0.87 | 39 | 0.76 | 39 | 0.73 | 39 | 0.73 | 44 |
| duration | 10000 | 0.19 | 38 | 0.09 | 39 | 0.08 | 39 | 0.08 | 45 |
| sessions | 10000 | 0.19 | 39 | 0.09 | 39 | 0.07 | 38 | 0.07 | 44 |
| baseline | 100000 | 1.52 | 39 | 0.51 | 39 | 0.38 | 39 | 0.45 | 106 |
| moving_points | 100000 | 2.16 | 39 | 1.12 | 39 | 0.98 | 39 | 1.02 | 106 |
| moving_minutes | 100000 | 8.45 | 39 | 7.42 | 39 | 7.15 | 39 | 7.18 | 106 |
| duration | 100000 | 1.70 | 39 | 0.68 | 39 | 0.56 | 39 | 0.58 | 106 |
| sessions | 100000 | 1.64 | 39 | 0.64 | 39 | 0.52 | 39 | 0.54 | 106 |
| baseline | 1000000 | 14.96 | 39 | 4.92 | 39 | 3.63 | 39 | 4.39 | 723 |
| moving_points | 1000000 | 22.12 | 38 | 11.12 | 39 | 9.62 | 38 | 9.94 | 724 |
| moving_minutes | 1000000 | 83.81 | 39 | 74.01 | 39 | 69.93 | 38 | 70.95 | 724 |
| duration | 1000000 | 16.87 | 38 | 6.70 | 39 | 5.37 | 38 | 5.92 | 724 |
| sessions | 1000000 | 16.31 | 38 | 6.29 | 39 | 4.99 | 39 | 5.37 | 724 |

## Memory against the window size

`tumbling_points`, input `gen`, after the change: consecutive blocks of k records.

| records | k | seconds | peak RSS MiB |
|---:|---:|---:|---:|
| 10000 | 10 | 0.18 | 39 |
| 10000 | 1000 | 0.17 | 39 |
| 100000 | 10 | 1.57 | 39 |
| 100000 | 1000 | 1.47 | 39 |
| 100000 | 100000 | 1.60 | 150 |
| 1000000 | 10 | 15.82 | 38 |
| 1000000 | 1000 | 14.65 | 38 |
| 1000000 | 100000 | 14.90 | 150 |

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
| never_closes | 1000 | 0.15 | 0.04 | 39 | 39 |
| never_closes | 2000 | 0.50 | 0.05 | 40 | 39 |
| never_closes | 4000 | 1.88 | 0.08 | 42 | 39 |
| never_closes | 8000 | 7.26 | 0.14 | 47 | 39 |
| never_closes | 16000 | 28.73 | 0.26 | 56 | 39 |
| never_closes | 100000 | - | 1.51 | - | 40 |
| never_closes | 1000000 | - | 15.12 | - | 61 |
| never_closes_read | 1000 | 0.15 | 0.04 | 39 | 39 |
| never_closes_read | 2000 | 0.50 | 0.05 | 40 | 40 |
| never_closes_read | 4000 | 1.86 | 0.08 | 42 | 42 |
| never_closes_read | 8000 | 7.30 | 0.15 | 47 | 46 |
| never_closes_read | 16000 | 29.09 | 0.27 | 56 | 56 |
| never_closes_read | 100000 | - | 1.59 | - | 152 |
| never_closes_read | 1000000 | - | 15.90 | - | 1185 |
| never_closes_start | 1000 | 0.20 | 0.21 | 39 | 39 |
| never_closes_start | 2000 | 0.72 | 0.74 | 40 | 40 |
| never_closes_start | 4000 | 2.80 | 2.82 | 42 | 42 |
| never_closes_start | 8000 | 11.05 | 11.14 | 47 | 47 |
| never_closes_start | 16000 | 43.68 | 44.51 | 56 | 56 |
| open_at_end | 1000 | 0.15 | 0.04 | 39 | 39 |
| open_at_end | 2000 | 0.50 | 0.05 | 40 | 38 |
| open_at_end | 4000 | 1.87 | 0.08 | 42 | 39 |
| open_at_end | 8000 | 7.32 | 0.14 | 47 | 39 |
| open_at_end | 16000 | 28.93 | 0.26 | 56 | 39 |
| open_at_end | 100000 | - | 1.52 | - | 40 |
| open_at_end | 1000000 | - | 15.19 | - | 61 |
| open_at_end_read | 1000 | 0.15 | 0.05 | 39 | 39 |
| open_at_end_read | 2000 | 0.52 | 0.09 | 40 | 40 |
| open_at_end_read | 4000 | 1.95 | 0.25 | 42 | 42 |
| open_at_end_read | 8000 | 7.70 | 0.96 | 47 | 47 |
| open_at_end_read | 16000 | 30.76 | 3.88 | 56 | 56 |
| first_stays_open | 1000 | 0.04 | 0.04 | 39 | 39 |
| first_stays_open | 2000 | 0.06 | 0.06 | 40 | 39 |
| first_stays_open | 4000 | 0.10 | 0.09 | 42 | 39 |
| first_stays_open | 8000 | 0.18 | 0.16 | 47 | 39 |
| first_stays_open | 16000 | 0.38 | 0.30 | 56 | 39 |
| tumbling_never | 1000 | 0.04 | 0.04 | 38 | 39 |
| tumbling_never | 2000 | 0.05 | 0.05 | 39 | 40 |
| tumbling_never | 4000 | 0.08 | 0.08 | 42 | 42 |
| tumbling_never | 8000 | 0.15 | 0.15 | 46 | 46 |
| tumbling_never | 16000 | 0.27 | 0.27 | 55 | 55 |
| tumbling_never | 100000 | - | 1.57 | - | 150 |
| tumbling_never | 1000000 | - | 15.55 | - | 1161 |

The two quadratic forms cannot be avoided by this change:

- `never_closes_start`: each window has its own value of the condition, n windows are
  open by the specification, and nothing in the condition proves that a window can no
  longer close, so n(n+1)/2 evaluations remain.
- `open_at_end_read`: n windows are returned by the specification and the query reads
  each of them: the sum of their sizes is 128008000 at 16000 records.

## `never_closes_start` across runners

An earlier pair of runs on two runners gave 45.71 s before and 67.10 s after at 16000
records. It does not reproduce on one runner. Run 41628529325, seconds of 3 rounds
over all the variants in one job, 16000 records:

| variant | `gen` | `array` |
|---|---|---|
| window code of `8b3cae940` | 34.20 36.60 34.27 | 31.83 31.54 31.39 |
| lazy input, deque | 34.83 34.73 34.24 | 31.35 31.34 31.33 |
| lazy input, vector | 34.82 34.47 34.21 | 30.90 31.97 31.12 |
| eager input, deque | 33.29 33.88 33.78 | 30.79 30.99 32.02 |
| eager input, vector | 33.91 34.85 33.96 | 30.84 32.28 31.63 |

In run 41635559360 it takes 43.68 s before and 44.51 s after, 2% more.

## Why, in the code

`src/runtime/core/gflwor/window_iterator.cpp`, `nextImpl`, buffers the binding sequence
in a temp sequence (`theDomainSeq`), and a window is a pair of positions in it.

- The temp sequence is lazy (`SimpleLazyTempSeq`), for sliding clauses as for tumbling
  ones, unless a side effect can run during the query, so that reading the input does not
  interleave with it. `expr_tools::may_run_side_effects` checks the whole query, following
  user functions: sequential exprs and functions, dynamic function calls, eval, and
  built-ins that run a function. A query with one of them keeps the eager input of
  sliding clauses, at the memory of the input. The check covers no tumbling clause:
  those read lazily in every query, as they did.
- After each returned window and after each item, `doGarbageCollection` purges up to
  the item before the start of the first open window, which `previous` may read, or up
  to the item before the current one if no window is open. It runs after the consumer
  has asked for the next tuple: `order by`, `group by` and function items copy the
  window by then.
- If nothing refers to the window variable, to the start variables or to the items of
  the end variables after the clause, no window needs items, and only the current item
  and its neighbors stay.
- An end condition is shared between windows, evaluated once per item, if it reads no
  start variable and repeats: built-in deterministic functions that take no function and
  return atomic values or items they received, and no constructor
  (`same_end_cond_for_all_windows`). Other conditions are evaluated per window.
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
