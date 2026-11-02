# Time and memory limits of recursive queries

Measured on GitHub Actions `ubuntu-24.04` runners, Release build
(`full-release`, Arch builder image), 2026-11-02. "Before" is master at
`3fd46f97f`, "after" is branch `timeout-recursive-functions`. Peak RSS comes from
`wait4`. Commands ran under an address-space limit so that "before" could not
exhaust the runner.

## Peak memory of a tree recursion

`declare function local:f($n){ if ($n le 0) then 0 else local:f($n - 1) + local:f($n - 1) }; local:f(n)`

| n | before, MB | after, MB |
|---:|---:|---:|
| 10 | 43 | 34 |
| 12 | 76 | 35 |
| 14 | 210 | 34 |
| 16 | 744 | 34 |
| 17 | 1456 | 34 |
| 18 | 2880 | 35 |
| 19 | 5728 | 35 |
| 20 | 6091 (limit 6 GB: `std::bad_alloc` at 7.1 s) | 34 |

The same shape through `let`, mutual recursion and function items is flat at 34
to 35 MB after. 800 chains of 900 calls: 316 MB before, 38 MB after.

Not covered: a call whose consumer stops early and does not ask for its end
(`head()`, a positional filter). `head(local:f($n - 1)) + head(local:f($n - 1))`
at n=18 still takes 1776 MB (3184 MB before). The time limit stops it, and in
`zorba-mcp` so does `--max-memory`.

## Time limit: how long after the limit the error comes

| Query, limit | before | after |
|---|---:|---:|
| tree recursion, n=20, 1 s (limit 3 GB) | 3.34 s, `std::bad_alloc` | 1.02 s |
| tree recursion, n=40, 1 s | 7.07 s, `std::bad_alloc` (6 GB) | 1.02 s |
| function items, n=40, 1 s | 59.95 s, `std::bad_alloc` (6 GB) | 1.02 s |
| nothing but a filter, 1 s | 1.03 s | 1.03 s |
| constant expression, 1 s | 2.37 s | 1.70 s |
| three nested constant expressions, 1 s | 7.11 s | 1.71 s |
| three nested constant expressions, 3 s (in test) | 21.5 s | about 3.7 s |
| external function of 500 ms per call, 1 s (in test) | 18.5 s | about 1.1 s |
| string function over 50 MB per step, 3 s (in test) | 69.3 s | about 3.1 s |
| `replace` over 100 MB per step, 3 s | not measured | 3.29 to 4.40 s |
| `order by` on 3 million strings, 2 s (in test) | 5.79 s | 2.33 s under ASan |
| `order by` on 10 million strings, 8 s, 3 GB held | not measured | 9.83 s |
| `zorba-mcp --timeout 2`, n=40 | killed or `std::bad_alloc` | 2.03 s, 36 MB |

So the error comes:

- milliseconds after the limit for a query that holds little memory;
- after the current step otherwise: one string operation (about 1.4 s for
  `replace` over 100 MB), one regular expression match (ICU's own limit stops a
  runaway match after about 0.6 s, with `zerr:ZXQP0001`);
- plus the time to free what the query holds: about 0.6 s per GB;
- plus up to 500 ms of constant folding before the limit starts, and the rest of
  compilation, which no limit covers.

## Cost

Same job, binaries alternating, median of 7 process runs. B = after. No
`--timeout` unless stated.

| Query | A, s | B, s | B/A | B with `--timeout 3600`, s | vs B |
|---|---:|---:|---:|---:|---:|
| tree recursion, n=16 | 0.989 | 0.228 | 0.23 | 0.232 | 1.02 |
| recursion from a loop, one call site | 0.404 | 0.397 | 0.98 | 0.420 | 1.06 |
| walk over 65535 nodes | 1.182 | 0.760 | 0.64 | 0.757 | 1.00 |
| recursion over a list, 900 levels | 1.528 | 1.491 | 0.98 | 1.528 | 1.02 |
| small function in a loop | 0.379 | 0.386 | 1.02 | 0.400 | 1.04 |
| two nested functions in a loop | 0.648 | 0.661 | 1.02 | 0.696 | 1.05 |
| three nested functions in a loop | 0.704 | 0.716 | 1.02 | 0.766 | 1.07 |
| function items, n=16 | 0.505 | 0.434 | 0.86 | 0.441 | 1.02 |
| filter | 1.582 | 1.614 | 1.02 | 1.650 | 1.02 |
| FLWOR | 0.738 | 0.745 | 1.01 | 0.771 | 1.03 |

With no time limit the suite below did not get slower. With a time limit,
queries that are almost nothing but function calls pay up to 7%: each call reads
the clock.

`bench.yml` (XMark and JSON suites, no time limit, A and B in the same job): at
the largest scale of each suite the wall time ratio B/A is at most 1.03, and peak
RSS is unchanged. Three XMark queries at scale 0.1 show 1.10 (q04, q13, q20:
0.20 s against 0.22 s); the same comparison run on the commit before showed them
at 1.00, and at scale 1 they are at most 1.02.

## Memory limit

`--max-memory` sets `RLIMIT_AS` in `zorba-mcp`, so it adds no cost per
allocation. It counts address space, which is somewhat more than the memory in
use, and exists on Linux only. An option on the security policy would need an
accounting allocator: a replaced global `operator new` with per-query counters,
reached from every allocation in the engine and its libraries.
