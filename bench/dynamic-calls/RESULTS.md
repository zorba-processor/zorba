# Cost of keeping dynamic function calls in place

A dynamic function call is an ordinary pure expression in a query with no side effect
reachable. In a query where something with a side effect is reachable, a call whose
function the compiler cannot follow stays where it is written, and nothing moves across
it. These queries call a pure function that the compiler cannot follow (`$fs[1]` of a
sequence of function items). `pure_*` have no side effect; `effects_*` have an assignment.

Measured on GitHub Actions `ubuntu-24.04` runners, Release build (`full-release`, Arch
builder image), 2026-11-03: `8b6dca1b1` (before) and `4875e63da` (after, the last commit
that changes the compiler), in the same job. Each cell is the median of 5 fresh processes,
at the default level O1. Peak RSS comes from `wait4`. Run 41724495511; `effects_streaming`
41729527825; `effects_binding_sequence` 41730786210. Each query gave the same result before and after.

| Query | What it does | Before | After | Ratio | Peak RSS before, after |
|---|---|---|---|---|---|
| `pure_hoistable` | 20000 iterations, 1000 items per call, constant argument | 0.028 s | 0.028 s | 1.00 | 36, 36 MiB |
| `pure_cheap` | 10^6 iterations, `$x + 1`, constant argument | 0.178 s | 0.181 s | 1.02 | 35, 36 MiB |
| `pure_beside_static` | a static call of 1000 items beside the call | 0.045 s | 0.044 s | 0.98 | 36, 36 MiB |
| `pure_per_item` | 10^6 iterations, argument from the loop | 0.900 s | 0.815 s | 0.91 | 35, 36 MiB |
| `pure_streaming` | 3 * 10^6 iterations, argument from the loop | 2.620 s | 2.383 s | 0.91 | 36, 36 MiB |
| `pure_constructor` | 3 * 10^6 iterations over constructed nodes | 4.081 s | 4.045 s | 0.99 | 36, 36 MiB |
| `window_pure_call` | a sliding window over 20000 strings of 16 KiB with a call | 0.348 s | 0.147 s | 0.42 | 425, 37 MiB |
| `effects_hoistable` | as `pure_hoistable`: the call runs per iteration | 0.028 s | 2.110 s | 75 | 36, 36 MiB |
| `effects_cheap` | as `pure_cheap`: the call runs per iteration | 0.180 s | 0.836 s | 4.6 | 36, 36 MiB |
| `effects_beside_static` | as `pure_beside_static`: nothing leaves the loop | 0.043 s | 2.092 s | 49 | 36, 36 MiB |
| `effects_per_item` | as `pure_per_item` | 0.846 s | 0.863 s | 1.02 | 36, 36 MiB |
| `effects_streaming` | as `pure_streaming`: the domain has no call, so it stays lazy | 2.523 s | 2.532 s | 1.00 | 36, 36 MiB |
| `effects_binding_sequence` | the domain of a `for` has the call: 3 * 10^6 items held | 2.590 s | 2.732 s | 1.05 | 36, 193 MiB |
| `proven_hoistable` | a function followed through a prolog variable | 0.027 s | 0.027 s | 1.00 | 36, 36 MiB |
| `proven_for_each` | `fn:for-each` with a named function reference | 0.028 s | 0.027 s | 0.96 | 36, 35 MiB |
| `no_dynamic_call` | control | 0.527 s | 0.527 s | 1.00 | 37, 37 MiB |

The ratios of `effects_hoistable`, `effects_cheap` and `effects_beside_static` depend on the
work per call and the number of iterations: it is what running a call in every iteration
costs, against once.

## Compile time

Whole process for generated queries that do little at run time, same runs and method. The
generator is not in the repository: `wide` has 500 functions and 300 FLWOR expressions of
30 let clauses each, `deep` one FLWOR expression of 600 let clauses, `nested` 150 nested
FLWOR expressions.

| Query | Before | After | Ratio |
|---|---|---|---|
| no dynamic call, wide | 0.171 s | 0.172 s | 1.01 |
| no dynamic call, deep | 0.048 s | 0.049 s | 1.02 |
| no side effect, a call in each FLWOR, wide | 0.300 s | 0.295 s | 0.98 |
| no side effect, a call, deep | 0.051 s | 0.049 s | 0.96 |
| side effect, a call in each FLWOR, wide | 0.300 s | 0.627 s | 2.1 (RSS 81 to 123 MiB) |
| side effect, a call, deep | 0.050 s | 0.126 s | 2.5 |
| side effect, a call, nested | 0.030 s | 0.038 s | 1.27 |

## Conservative rule

The tables above are for `4875e63da`. Later commits changed which hoisting is allowed
beside a call that can have a side effect, and the final rule is the conservative one: in
a query where something with a side effect is reachable and a call the compiler cannot
prove free of side effects appears anywhere, no body hoists anything and no index join is
made. Such a call can update a node or JSON item that a path or a lookup reads, also in a
function whose result it consumes lazily, and finer rules that keep hoisting next to it rely on the store rejecting, in the modify clause of a
transform, every update that does not target a copy (XUDY0014, XUDY0037), which it does
not do yet. In a query whose calls through function items are all proven, an expression
that reads a state such a call can change stays in its loop, and other expressions are
hoisted. A query with no side effect
is not affected.

Measured with `bench.yml` in suite mode (`bench/compare.sh`), run 42223320339, 2026-11-06:
base `8b6dca1b1` (merge base), head `af5f34d5c` (the last commit that changes the
compiler), Release builds measured in the same job, median of 5 timed runs after
a warm-up, at O1. 19 queries, 0 failed, every output the same. The runner was
faster than the one of the first table, so compare ratios, not times. `effects_beside_static`
is back to its cost in the first table: the static call beside the call stays in the loop.
Calls the compiler follows (`effects_union_invariant`, `effects_lookup_invariant`,
`proven_*`) and pure queries keep their hoisting.

| Query | Base median | Head median | Ratio | Peak RSS base, head |
|---|---|---|---|---|
| `effects_beside_static` | 0.032 s | 1.039 s | 32.98 | 36.0, 36.2 MiB |
| `effects_binding_sequence` | 1.547 s | 1.759 s | 1.14 | 35.6, 193.1 MiB |
| `effects_cheap` | 0.099 s | 0.420 s | 4.26 | 35.7, 35.7 MiB |
| `effects_hoistable` | 0.021 s | 1.019 s | 47.83 | 35.7, 35.8 MiB |
| `effects_lookup_invariant` | 0.030 s | 0.030 s | 1.01 | 35.7, 35.9 MiB |
| `effects_per_item` | 0.424 s | 0.431 s | 1.02 | 35.7, 35.8 MiB |
| `effects_streaming` | 1.221 s | 1.276 s | 1.04 | 35.7, 35.9 MiB |
| `effects_union_invariant` | 0.029 s | 0.030 s | 1.00 | 36.0, 35.9 MiB |
| `no_dynamic_call` | 0.521 s | 0.521 s | 1.00 | 37.5, 37.3 MiB |
| `proven_for_each` | 0.021 s | 0.022 s | 1.03 | 35.7, 35.5 MiB |
| `proven_hoistable` | 0.021 s | 0.022 s | 1.01 | 35.6, 35.9 MiB |
| `pure_beside_static` | 0.030 s | 0.030 s | 0.98 | 35.9, 36.1 MiB |
| `pure_cheap` | 0.099 s | 0.098 s | 0.98 | 35.7, 35.9 MiB |
| `pure_constructor` | 2.175 s | 2.225 s | 1.02 | 35.7, 35.9 MiB |
| `pure_fold_hoistable` | 0.038 s | 0.038 s | 0.99 | 35.7, 36.0 MiB |
| `pure_hoistable` | 0.022 s | 0.022 s | 0.97 | 35.7, 35.9 MiB |
| `pure_per_item` | 0.454 s | 0.399 s | 0.88 | 35.6, 35.9 MiB |
| `pure_streaming` | 1.301 s | 1.166 s | 0.90 | 35.8, 35.9 MiB |
| `window_pure_call` | 0.226 s | 0.076 s | 0.34 | 424.8, 36.7 MiB |

These classes of operations no longer count as code that creates nodes, which keeps
hoisting in queries with no side effect and lets more calls be proven: a union, intersect, except, the distinct-node operators, `fn:id`, `fn:idref`,
`fn:element-with-id`, single JSON lookups, `fn:doc`, `fn:collection` and calls of
`fn:fold-left`, `fn:fold-right`, `fn:for-each-pair` with a known function.
The compile-time table above was not measured again.
