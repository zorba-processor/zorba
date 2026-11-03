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
