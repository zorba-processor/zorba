# Index probe bound with let in a function body

`udf_let_probe.xq` loads 10000 `item` elements into a collection with an automatic
value index on `@k`, then calls 200000 times a sequential function shaped like
`data:remove-document` of `test/rbkt/Queries/zorba/index/match_veq_06.xqlib`:
`let $hit := for $i in collection where $i/@k eq $k return $i`. Index matching turns
the `for` into an index probe. Before `fix-udf-identity-flwor` the FLWOR
`for $i in probe return $i` stayed around the probe; after it, the let binds the probe.

## Results

`bench.yml` with `suite=index-probe`, run 43543734002, 2026-11-14, GitHub Actions
`ubuntu-24.04` runner (AMD EPYC 9V45, 4 CPUs), Release build (`full-release`), both
builds in the same job: base `523bdc3dbe` (master), head `79e87fdd7a`. One warm-up run,
then 7 timed runs of each, alternating. Both builds wrote the same output.

| Query | Base median | Base min | Head median | Head min | Median head/base | Min head/base | Peak RSS base, head |
|---|---|---|---|---|---|---|---|
| `udf_let_probe` | 0.382 s | 0.361 s | 0.324 s | 0.294 s | 0.85 | 0.82 | 89.1, 89.3 MiB |

`b:lookup` is declared `%an:sequential`, so it is not inlined: `InlineFunctions` skips a
function whose `isSequential()` is true, and the declaration sets that (the body only
draws warning ZWST0003).

## Compile time

The extra rewrite (`MarkExprs`, then `EliminateUnusedLetVars` once) runs only when an
index matched in the expression being optimized. A query or function without a match
pays one boolean test. Not measured separately.
