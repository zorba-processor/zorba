# Index probe bound with let in a function body

`udf_let_probe.xq` loads 10000 `item` elements into a collection with an automatic
value index on `@k`, then calls 200000 times a sequential function shaped like
`data:remove-document` of `test/rbkt/Queries/zorba/index/match_veq_06.xqlib`:
`let $hit := for $i in collection where $i/@k eq $k return $i`. Index matching turns
the `for` into an index probe. Before `fix-udf-identity-flwor` the FLWOR
`for $i in probe return $i` stayed around the probe; after it, the let binds the probe.

## Results

`bench.yml` with `suite=index-probe`, run 43501550670, 2026-11-13: base `20d5787a54`
(master), head `d88b2b5de0`. Both builds ended the warm-up run with signal 11
(`exit-11`), so there is no timing. The crash happens before and after the change; its
cause is not known yet.

## Compile time

The extra rewrite (`MarkExprs`, then `EliminateUnusedLetVars` once) runs only when an
index matched in the expression being optimized. A query or function without a match
pays one boolean test. Not measured separately.
