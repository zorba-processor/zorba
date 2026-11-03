# Cost of keeping dynamic function calls in their loops

A call is hoisted only if the compiler can see the function and prove it deterministic
and free of side effects. These queries measure what that costs where the function is
pure but the compiler cannot see it (`$fs[1]` of a sequence of function items).

Measured on one GitHub Actions `ubuntu-24.04` runner, Release build (`full-release`, Arch
builder image), 2026-11-03: `8b6dca1b1` (before) and the head of this change (after), in
the same job. Each cell is the median of 5 fresh processes, at the default level O1. Peak
RSS comes from `wait4`. Runs 41676604089, and 41678316604 for `unproven_beside_static`.
Each query gave the same result before and after.

| Query | Calls | Before | After | Ratio | Peak RSS after |
|---|---|---|---|---|---|
| `unproven_hoistable` | 20000 iterations, 1000 items per call | 0.023 s | 1.683 s | 73 | 36 MiB |
| `unproven_cheap` | 10^6 iterations, `$x + 1` | 0.139 s | 0.616 s | 4.4 | 36 MiB |
| `unproven_beside_static` | 20000 iterations, a static call of 1000 items beside the call | 0.042 s | 2.081 s | 50 | 36 MiB |
| `unproven_per_item` | 10^6 iterations, argument from the loop | 0.625 s | 0.630 s | 1.01 | 36 MiB |
| `streaming_unproven_memory` | 3 * 10^6 iterations, argument from the loop | 1.734 s | 1.853 s | 1.07 | 36 MiB (35 before) |
| `proven_hoistable` | a function the compiler follows through a prolog variable | 0.023 s | 0.022 s | 0.96 | 36 MiB |
| `proven_for_each` | `fn:for-each` with a named function reference | 0.022 s | 0.023 s | 1.05 | 35 MiB |
| `no_dynamic_call` | control, no dynamic call | 0.522 s | 0.523 s | 1.00 | 38 MiB |

The ratio of the first three depends on the work per call and the number of iterations:
it is what running the call or its neighbour in every iteration costs, against once.
A call whose argument depends on the loop was never hoisted, so it loses nothing but
the 1% to 7% above. A loop where only one clause makes such a call and the others read
no state stays lazy: peak RSS does not grow with the input.
