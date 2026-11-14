# Index probe bound with let in a function body

`udf_let_probe.xq` loads 10000 `item` elements into a collection with an automatic
value index on `@k`, then calls 200000 times a sequential function shaped like
`data:remove-document` of `test/rbkt/Queries/zorba/index/match_veq_06.xqlib`:
`let $hit := for $i in collection where $i/@k eq $k return $i`. Index matching turns
the `for` into an index probe. Before `fix-udf-identity-flwor` the FLWOR
`for $i in probe return $i` stayed around the probe; after it, the let binds the probe.

## Results

Not measured yet. Run 43501550670 stopped with XSST0006 (the sequential call was in a
`where` clause), run 43523424503 with ZDTY0001 (the constructed items were typed
`xs:anyType`; the library now declares `construction strip`), and run 43535397986 with
XPTY0004 (the value of the last statement of a block is not the value of the block; the
function now returns the `let` expression), in both builds.

## Compile time

The extra rewrite (`MarkExprs`, then `EliminateUnusedLetVars` once) runs only when an
index matched in the expression being optimized. A query or function without a match
pays one boolean test. Not measured separately.
