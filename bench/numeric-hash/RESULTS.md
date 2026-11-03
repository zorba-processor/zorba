# Hashing numeric keys against xs:float

`16777217 eq xs:float(16777216)` is true (F&O 3.0 op:numeric-equal, XPath 3.0 B.1:
the integer is cast to xs:float), so `fn:distinct-values` and `group by` must merge
the two, and their hash codes must agree.

F&O 3.0 fn:distinct-values: "If the input sequence contains values of different
numeric types that differ from each other by small amounts, then the eq operator is
not transitive, because of rounding effects occurring during type promotion. In the
situation where the input contains three values A, B, and C such that A eq B, B eq C,
but A ne C, then the number of items in the result of the function (as well as the
choice of which items are returned) is implementation dependent, subject only to the
constraints that (a) no two items in the result sequence compare equal to each other,
and (b) every input item that does not appear in the result sequence compares equal
to some item that does appear in the result sequence." XQuery 3.0 3.10.7 states the
same constraints for `group by`.

So the merge is required, and which one survives is free.

## Designs measured

- master: `Item::hash()`, wrong for these pairs, the baseline for speed.
- Obvious fix: every integral value above 2^24 hashes on its xs:float rounding, in
  every container. Commit 9f6fbf75b (branch `nsp-obvious-hash`, measurement only).
- This change: a container starts with `Item::hash()` and moves to the xs:float
  rounding at its first xs:float key (`src/zorbautils/numeric_key_hash.h`).

Up to 2^(e-23) integers around 2^e round to one xs:float: 512 around 2^32, 131072
around 2^40.

## Results

`bench.yml`, one timed run after a warm-up, same job for both sides; seconds of wall
time at the 100mb scale (these queries do not read the data). Runs 41536883606
(obvious), 41536817077 and 41549111041 (this change).

| Query | Keys | master | Obvious fix | This change |
|---|---|---:|---:|---:|
| j9 | 1000000 integers above 2^32 | 0.30 | 17.76 (58.8x) | 0.30 (1.00x) |
| j10 | 20000 integers above 2^40 | 0.04 | 13.64 (336x) | 0.04 (1.00x) |
| j11 | j10 with `group by` | 0.04 | 4.57 (113x) | 0.04 (1.00x) |
| j12 | j9, one xs:float key last | 0.26 | 17.67 (67x) | 0.93 (3.5x) |
| j13 | j9, one xs:float key first | 0.26 | not run | 21.14 (81x) |

The cost of this change is j12 and j13: a container with an xs:float key among large
integers pays what the obvious fix pays on all of them. j12 is one rehash; j13 is the
bucket sharing from the start, plus the conversion of each key to xs:float. j14 in
`bench/json` does not measure this: all its integers equal its xs:float key, so each
lookup ends at the first comparison.

A hash join on operands of static type xs:anyAtomicType makes a second index probe
for each outer tuple, to see whether the inner side holds an xs:float; with one, or
with an xs:float outer key, that probe scans the inner side. j4_join against master:
6.65 s against 6.48 s and 6.58 s against 6.66 s at 100mb, in two runs. One
run each, so this does not separate a cost of a few percent from runner noise. The
highest XMark ratios are 1.10 (q04 in the first run, q03 and q13 in the second), each
in one run only.

## Not measured

The fallback of a container that holds a value rounding to different xs:float values
directly and through xs:double (2^60 + 2^36 + 1, or a long decimal near a midpoint of
two xs:float values): such a value equals an xs:float and an xs:double that differ,
so no hash of the rounding is consistent with eq, and all numeric keys of that
container share a bucket. Lookups are linear in the numeric keys of that container.
It requires an xs:float key and such a value in the same container.
