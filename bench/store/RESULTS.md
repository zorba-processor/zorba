# Store memory: results

## After sharing JSON keys (candidates 1 and 3)

Run 41446148403 (same job, inputs and settings as the baseline below, run
41377872156), on the commit before the loader's leak fix, which touches only
the path where a duplicate key raises an error. Objects of one parse share
their key items, and objects of at most 16 pairs have no `unordered_map`.

| format | file MiB | heap MiB before | heap MiB after | change | heap / file before | heap / file after | parse ms before | parse ms after | change |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| json | 1.0 | 21.0 | 9.3 | -55.9% | 20.7 | 9.1 | 66 | 58 | -11.8% |
| json | 10.3 | 210.0 | 92.8 | -55.8% | 20.4 | 9.0 | 659 | 576 | -12.5% |
| json | 104.4 | 2100.4 | 927.7 | -55.8% | 20.1 | 8.9 | 6605 | 5791 | -12.3% |
| csv | 1.0 | 16.5 | 10.5 | -36.6% | 15.8 | 10.0 | 49 | 44 | -11.4% |
| csv | 10.8 | 165.0 | 104.5 | -36.7% | 15.2 | 9.7 | 504 | 436 | -13.5% |
| csv | 111.9 | 1655.9 | 1050.7 | -36.5% | 14.8 | 9.4 | 5061 | 4412 | -12.8% |
| xml | 1.0 | 16.0 | 16.0 | 0.0% | 15.7 | 15.7 | 34 | 35 | +2.5% |
| xml | 10.4 | 160.3 | 160.3 | 0.0% | 15.4 | 15.4 | 343 | 343 | -0.1% |
| xml | 106.0 | 1603.3 | 1603.3 | 0.0% | 15.1 | 15.1 | 3420 | 3420 | 0.0% |

Times are medians of 3 runs from two different CI runs. XML goes through none
of the changed code and is the control: its time is unchanged at 10 and
100 MiB. CSV gains from the small objects only; it already shared keys.

Per kind (microbenchmarks, same derivation as the table below): `SimpleJSONObject`
112 -> 64 B, an empty object 136 -> 88 B. In `j_obj4`, a pair with its integer
value costs 240.4 -> 68.3 B: 48 B for the integer and 20 B of pair vector.
Live allocations per JSON item 3.43 -> 1.33. `Item::mem_size()` now reports
1.16x the JSON heap, up from 0.60x, because it counts a shared key item once
per object.

### Key index threshold

Throwaway builds read the threshold from the environment. Objects of k keys
(`key0`, `key1`, ...), 1.2 M pairs in all, median of 3 runs per point. "Index"
is threshold 0, "linear" is no index at any size; both share key items. Lookup
is the extra time of a query that looks up every key of every object with a
fresh string (mostly query overhead). Run 41443973653.

| k | parse ms index | parse ms linear | linear / index | lookup ms index | lookup ms linear | linear / index |
|---:|---:|---:|---:|---:|---:|---:|
| 1 | 1250 | 1018 | 0.81 | 248 | 210 | 0.85 |
| 2 | 994 | 879 | 0.88 | 606 | 536 | 0.88 |
| 4 | 868 | 822 | 0.95 | 556 | 484 | 0.87 |
| 8 | 795 | 775 | 0.97 | 542 | 543 | 1.00 |
| 12 | 803 | 787 | 0.98 | 537 | 541 | 1.01 |
| 16 | 767 | 766 | 1.00 | 552 | 559 | 1.01 |
| 20 | 763 | 778 | 1.02 | 600 | 558 | 0.93 |
| 24 | 770 | 775 | 1.01 | 599 | 611 | 1.02 |
| 32 | 746 | 788 | 1.06 | 592 | 604 | 1.02 |
| 48 | 756 | 841 | 1.11 | 586 | 652 | 1.11 |
| 64 | 736 | 892 | 1.21 | 593 | 693 | 1.17 |

The threshold is 16, the largest size measured where the linear search is not
slower to build. No index saves 61-144 B per pair at every size in this sweep,
so the limit comes from time, not memory.

Objects just above the threshold pay for 16 linear inserts and then for
building the index. Threshold 16 against an index from the first pair, median
of 5 runs (run 41446159839):

| k | parse ms index | parse ms threshold 16 | ratio | heap B / pair index | heap B / pair threshold 16 |
|---:|---:|---:|---:|---:|---:|
| 16 | 599 | 561 | 0.94 | 134.5 | 70.5 |
| 17 | 606 | 631 | 1.04 | 147.3 | 153.9 |
| 18 | 600 | 622 | 1.04 | 144.4 | 150.7 |
| 20 | 589 | 608 | 1.03 | 139.6 | 145.2 |
| 24 | 589 | 596 | 1.01 | 140.3 | 137.0 |
| 32 | 572 | 583 | 1.02 | 129.2 | 126.7 |
| 48 | 575 | 577 | 1.00 | 137.2 | 134.2 |

Objects of 17-20 pairs build up to 4% slower and take up to 4.5% more heap than
an index from the first pair. That comparison is within the new code; these
sizes were not measured on the baseline, so their cost against it is unknown.

Limits of these measurements:
- All keys are short and of nearly equal length. Keys of equal length with a
  long common prefix make each comparison in the linear search slower; that
  was not measured.
- A parse shares at most 1024 distinct keys of at most 256 bytes; other keys
  get their own item, as before. Documents past those limits were not measured.

## Baseline

Run 41377872156 (job `store` of `.github/workflows/directory-scaling.yml`,
`suite=store`, defaults: sizes 1 10 100 MiB, 3 runs per point, microbenchmark
N = 200000), ubuntu-24.04, full-release build, glibc malloc. Inputs come
from `gen.xq`; each file is parsed with the expression `dir:documents` uses
(`parse-xml(file:read-text())`, `jn:parse-json(...)`,
`[ csv:parse(...) ]`). The three runs of every point gave the same heap to
within 2 KB, so medians and single runs are the same.

How to read the numbers: *heap* is the live malloc memory the parsed result
holds (glibc `mallinfo2`, chunk headers and rounding included); *RSS* and
*peak* are the resident set after parsing and its maximum while parsing,
each minus the value before parsing. Chunk arithmetic below uses glibc's
rule: a request of n bytes takes max(32, round16(n + 8)) bytes.

## Totals

| format | file MiB | heap MiB | heap / file | RSS / file | peak / file | items | heap B / item | allocations / item | parse s |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| xml | 1.0 | 16.0 | 15.7 | 15.6 | 15.9 | 119,904 | 140.1 | 2.12 | 0.03 |
| xml | 10.4 | 160.3 | 15.4 | 15.4 | 15.5 | 1,198,860 | 140.2 | 2.12 | 0.34 |
| xml | 106.0 | 1603.3 | 15.1 | 15.1 | 15.2 | 11,988,573 | 140.2 | 2.12 | 3.42 |
| json | 1.0 | 21.0 | 20.7 | 20.6 | 20.8 | 110,567 | 199.1 | 3.43 | 0.07 |
| json | 10.3 | 210.0 | 20.4 | 20.4 | 20.5 | 1,105,463 | 199.2 | 3.43 | 0.66 |
| json | 104.4 | 2100.4 | 20.1 | 20.1 | 20.2 | 11,054,549 | 199.2 | 3.43 | 6.61 |
| csv | 1.0 | 16.5 | 15.8 | 15.6 | 15.8 | 102,353 | 169.2 | 2.38 | 0.05 |
| csv | 10.8 | 165.0 | 15.2 | 15.2 | 15.3 | 1,023,521 | 169.0 | 2.38 | 0.50 |
| csv | 111.9 | 1655.9 | 14.8 | 14.7 | 14.7 | 10,235,129 | 169.6 | 2.38 | 5.06 |

Items: XML nodes (document, elements, attributes, texts); JSON items
(objects, arrays, atomic values; object pairs not counted as items).

- The cost is linear in the input: 15x the file for XML and CSV, 20x for
  JSON, from 1 to 100 MiB. RSS after parsing equals the heap, and the peak
  while parsing is at most 1% above it, so the input text read by
  `file:read-text()` is not what limits the size; the parsed tree is.
- The 23x of the directory benchmark (#119) is in this range: its records
  are four short fields, where the fixed per-item costs weigh more.
- `Item::mem_size()`, the store's own estimate, is not usable as a meter:
  it reports 1.09x the heap for XML (it counts the shared element name in
  every node) and 0.60x for JSON (it misses hash-map nodes, malloc headers
  and the decimals' heap).

## Where it goes

Per-kind costs from the microbenchmarks (documents of 200000 items of one
kind; differences between cases), multiplied by the item counts of the
100 MiB files. The model explains the measured heap to within 3% (XML,
JSON) and 7% (CSV).

| kind | heap B each | allocations each | explanation (sizeof from `sizes.cpp`) |
|---|---:|---:|---|
| element | 120 | 1 | ElementNode 96 B, chunk 112, + 8 B slot in the parent's child vector |
| text node | 144 | 3 | TextNode 48 (chunk 64), its zstring rep (24 B + text + 1, chunk >= 32), the parent's child vector (chunk >= 32), slot |
| text, per character | 1.0 | 0 | |
| attribute | 208 | 4 | AttributeNode 64 (chunk 80), typed value UntypedAtomicItem 32 (chunk 48), its rep (>= 32), the element's node vector, slot |
| distinct name | 216 | 1.8 | QName pool entry; 10 distinct names per file, negligible |
| JSON object | 136 | 1 | SimpleJSONObject 112 (includes a 56 B `unordered_map` and the pair vector), chunk 128, + slot |
| JSON pair (key) | 182 | 3.5 | key StringItem + rep, `unordered_map` node + bucket array, 16 B pair slot |
| CSV pair (key) | 86 | | same objects; csv:parse reuses one key item per column |
| integer, double | 58.5 | 1 | IntegerItem 32 / DoubleItem 40, chunk 48, + slot; no cache for small values (`j_int` = `j_bigint`) |
| decimal | 202.5 | 1 | DecimalItem 32 (chunk 48) + 144 B that MAPM allocates with malloc (measured: heap minus operator-new blocks / decimals = 144.0 B for JSON and CSV) |
| string | 106.5 | 2 | StringItem 32 (chunk 48) + rep (>= 32) + slot |
| string, per character | 1.0 | 0 | |
| boolean, null | 8 | 0 | shared singletons: only the slot |

Breakdown of the 100 MiB files (share of the measured heap):

| XML (1603 MiB) | MiB | share | JSON (2100 MiB) | MiB | share | CSV (1656 MiB) | MiB | share |
|---|---:|---:|---|---:|---:|---|---:|---:|
| text nodes (4.94 M) | 678 | 42% | object pairs (7.37 M) | 1278 | 61% | object pairs (8.96 M) | 736 | 44% |
| element nodes (5.64 M) | 646 | 40% | strings (2.63 M) | 267 | 13% | strings (3.84 M) | 390 | 24% |
| attribute nodes (1.41 M) | 280 | 17% | integers (4.21 M) | 235 | 11% | decimals (1.28 M) | 247 | 15% |
| text characters | 46 | 3% | objects (1.05 M) | 137 | 7% | objects (1.28 M) | 166 | 10% |
| attribute characters | 4 | 0% | decimals (0.53 M) | 102 | 5% | integers (2.56 M) | 143 | 9% |
| | | | arrays (1.05 M) | 72 | 3% | string characters | 73 | 4% |
| | | | string characters | 31 | 1% | booleans | 10 | 1% |
| | | | doubles, booleans, nulls | 37 | 2% | | | |
| model minus measured | -50 | -3% | | -59 | -3% | | -108 | -7% |

malloc headers alone (8 B x live operator-new blocks, from the run's raw
counters): XML 25.4 M blocks = 194 MiB (12%), JSON 37.9 M = 289 MiB (14%),
CSV 24.3 M = 186 MiB (11%).

### Top contributors

1. **Per-item fixed overhead, not content.** Text content costs 1 B per
   character, but every text node costs 144 B and every element 120 B; the
   XML file's text is 50 MiB of its 106 MiB, yet characters are 3% of the
   heap. Average text node: 10.7 characters, 144 B of structure.
2. **JSON object keys stored per object.** 7.37 M pairs with 14 distinct
   keys cost 182 B each (61% of the JSON heap); the same structure from
   csv:parse, which shares the key items, costs 86 B per pair.
3. **One heap allocation per atomic value and per node, each with an 8 B
   malloc header**: 2.1 (XML) to 3.4 (JSON) blocks per item; headers alone
   are 11-14% of the heap. Integers and doubles are 48 B chunks to hold 8 B
   values; decimals add 144 B of MAPM heap each.

## Optimization candidates (ranked by estimated saving on these inputs)

Savings are for the 100 MiB files. "measured" means the per-item difference
comes from two measured cases; "estimate" means it comes from sizeof and
glibc chunk arithmetic and has not been measured.

| # | candidate | basis | XML | JSON | CSV |
|---|---|---|---:|---:|---:|
| 1 | Done, with 3: JSON -1173 MiB (-56%), CSV -605 MiB (-37%), see above. Share key items across the objects of one parse (as csv:parse does) | measured: JSON pair 182 B vs CSV pair 86 B = 96 B/pair | - | 673 MiB (32%) | already done |
| 2 | Per-document arena (bump) allocation of nodes and items: no per-block header or rounding | measured headers only (8 B x blocks); rounding not measured | >= 194 MiB (12%) | >= 289 MiB (14%) | >= 186 MiB (11%) |
| 3 | Done, with 1. Small-object JSON keys without `unordered_map` (linear search over the pair vector for small objects) | estimate: map node 32 B (chunk 48) + bucket array per key | - | ~340 MiB (16%) | ~410 MiB (25%) |
| 4 | Cached small integers (done, see "Shared small integers"; immediate items and doubles ruled out there) | measured on a modified store: 48.0 B per shared integer | - | 169 MiB (8.0%) | 59 MiB (3.5%) |
| 5 | Text stored inline in TextNode (small-string), no separate rep | estimate: rep chunk >= 32 B per text node | 151 MiB (9%) | - | - |
| 6 | Pack ElementNode (96 B) to 88 B | done, see "XML node packing"; measured: chunk 112 -> 96 | 86 MiB (5%) | - | - |
| 7 | Decimals without MAPM heap for values that fit (e.g. int64 mantissa + scale) | measured: 144 B per decimal outside operator new. **Done differently** (see "Compact decimals" below): 96 of the 144 B saved | - | 48 MiB (2%) measured | 117 MiB (7%) measured |
| 8 | Short strings inline in StringItem (no rep) | estimate: rep chunk >= 32 B per string | - | 80 MiB (4%) | 117 MiB (7%) |
| 9 | Attribute value as a plain string instead of an UntypedAtomicItem | not done, see "XML node packing"; estimate: 48 B chunk per attribute | 65 MiB (4%) | - | - |
| 10 | Child vector inline for elements with up to two attributes and children | done, see "XML node packing"; measured: vector chunk 32 B gone for 7 of 8 elements | 151 MiB (9%) | - | - |

The savings overlap (2 shrinks what 4-9 save; 1 and 3 both act on pairs) and
do not add up.

Not contributors on these inputs:
- OrdPath: 8 B embedded in the node (`OrdPath` sizeof 8) and no extra
  allocation (an empty element is exactly 1 block); deep documents, where
  OrdPaths spill to the heap, were not measured.
- QName pool: 216 B per distinct name, 10 names per file.
- vtable and reference count: `store::Item` is 24 B (vptr, refcount, union)
  in every item; removing the refcount (8 B) rarely changes the chunk class
  and is not estimated.
- Whitespace-only text nodes: 705 K of the 4.94 M text nodes (97 MiB, 6%);
  dropping them changes the data model, so it is an opt-in, not an
  optimization.

## Shared small integers

`BasicItemFactory::createInteger` returns one shared item per xs:integer value
from -128 to 1023. Run 41442221289 against the baseline above, 100 MiB files:

| format | heap MiB before | after | saved | live blocks saved | integers | shared | parse s before | after |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| xml | 1603.3 | 1603.3 | 0 | 0 | 0 | 0 | 3.42 | 2.67 |
| json | 2100.4 | 1931.7 | 168.7 (8.0%) | 3,685,872 | 4,211,256 | 3,685,872 (87.5%) | 6.61 | 5.37 |
| csv | 1655.9 | 1597.2 | 58.6 (3.5%) | 1,280,414 | 2,558,782 | 1,280,414 (50.0%) | 5.06 | 4.09 |

- This was measured before JSON keys were shared and decimals compacted. The
  blocks saved do not depend on either; against the heap of "After sharing JSON
  keys" (927.7 and 1050.7 MiB) the same bytes would be 18% and 5.6%. That is
  arithmetic, not a measurement of the combined store.
- Live blocks saved equal the shared integers exactly, and the heap saved is
  48.0 B per shared integer in both formats: the 48 B chunk goes, the 8 B slot
  stays. The saving is below the 203 MiB + 123 MiB estimate of candidate 4
  because that estimate assumed every integer shared and 50.5 B each.
- The heap column is measured after minus before parsing, so it does not show
  the table itself: about 64 KB per store (1152 items of 48 B, 1152 handles of
  8 B), by arithmetic.
- **Time is not comparable between the two runs.** They ran on different
  runners, and work the change cannot affect also got faster: XML -22%,
  `x_empty` -26%, `j_arr0` -10%, `j_bool` -19%. JSON (-19%) and CSV (-19%) are
  inside that spread, so these runs show no time regression above 5% but do not
  measure a gain or exclude a small loss. Relative to XML, JSON is +4% and CSV
  +4%. `j_bigint`, which pays the range check and never hits, moved with the
  controls (-18%); `j_int`, which always hits, moved more (-33%).

Limits of these numbers:
- The hit rates come from `gen.xq`. Its small fields are `v` (0..999), `pt.x`,
  `pt.y` and `nums`; only `id` is mostly outside the range. The upper bound 1023
  is what covers `v`: with -128..127 the shares would be about 77% for JSON and
  6% for CSV. No real-world integer distribution was measured, and the benchmark
  has no negative integers, so -128..-1 is a judgment, not a result.
- The engine is single-threaded by default (`ZORBA_FOR_ONE_THREAD_ONLY=ON`, see
  `doc/zorba/threading.dox`): reference counts are plain integers, with no lock.
  Concurrent use without the application's own mutex is already undefined, but
  it now fails sooner: threads taking references to the same small value crash
  the process (seen in an earlier version of the test, run 41444048529).
- Big-integer builds were tested, not measured. A hit there
  converts the value through a string to find the table slot.

Rulings:
- Immediate (tagged) items: out of scope. `store::Item_t` and every caller
  assume a real object with a vtable and a reference count.
- Derived integer types (xs:long ... xs:byte, xs:nonNegativeInteger, ...) are
  not cached: the JSON and CSV parsers and xs:integer arithmetic do not produce
  them, and each would need its own table for its own type code.
- Doubles are not cached. The JSON file has 526,407 (CSV: none). A table of
  integral values in the same range would hold one of the 1000 values `gen.xq`
  produces (500.0). A cache keyed by value, needing a hash lookup per double,
  could save at most 24 MiB (1.2% of the JSON heap), and even that depends on
  the repetition `gen.xq` builds in with `$i mod 1000`.

## Compact decimals (candidate 7)

The tables above describe master before this change. A decimal's MAPM number
was two blocks: `M_APM_struct` (40 B, chunk 48) and a digit array (84 B, chunk
96). Now `M_APM_struct` is 32 B, a new number is one block (struct + 84 B,
chunk 128), and `DecimalItem` keeps a copy in one block sized to fit (struct +
digits rounded up to 8 bytes: 40 B, chunk 48, up to 16 significant digits).

Measured with `suite=store`: run 41444652502 (change) against run 41444656225
(master plus the `j_dec_arith` case), 3 runs per point, medians:

| | before | after | change |
|---|---:|---:|---:|
| CSV heap, 100 MiB file | 1655.9 MiB | 1538.7 MiB | -117.2 MiB (-7.1%) |
| JSON heap, 100 MiB file | 2100.4 MiB | 2052.3 MiB | -48.1 MiB (-2.3%) |
| XML heap, 100 MiB file | 1603.3 MiB | 1603.3 MiB | 0 |
| decimal, heap per item (`j_dec`) | 202.5 B | 106.5 B | -96 B |
| CSV parse, 100 MiB file | 4.98 s | 5.00 s | +0.4% |
| JSON parse, 100 MiB file | 6.58 s | 6.61 s | +0.4% |
| `j_dec` parse, 200000 decimals | 94 ms | 83 ms | -12% |
| `j_dec_arith`, same document, whole query | 235 ms | 230 ms | -2% |
| `j_dec_arith` minus `j_dec` (arithmetic only) | 141 ms | 147 ms | +4% |

XML, which has no decimal items, moved 2.1% between the two runs (3.54 s and
3.47 s), so differences of that size are runner noise. `j_dec_arith` is
`sum( for $d in jn:members( jn:parse-json( $s ) ) return $d * 1.5 + 0.25 )`:
every intermediate result is a decimal item, so it pays for one compaction per
item, and a new number saves one allocation.

An int64 mantissa plus scale inside the item, as first proposed, would save
the other 48 B per decimal (59 MiB on CSV, 24 MiB on JSON; arithmetic, not
measured), but every `getDecimalValue()`, now a reference count increment,
would have to build a MAPM number, and values that do not fit would need a
second representation. The compact block keeps one representation, so results
cannot differ between forms.

## XML node packing

The tables above describe master before this change (run 41377872156). Run
41448453817 measures the store with candidates 6 and 10 applied; same inputs
and settings. JSON and CSV file heaps are unchanged to the digit.

| XML file MiB | heap MiB before | after | heap / file before | after | allocations / item before | after |
|---:|---:|---:|---:|---:|---:|---:|
| 1.0 | 16.0 | 13.7 | 15.7 | 13.4 | 2.12 | 1.71 |
| 10.4 | 160.3 | 136.7 | 15.4 | 13.1 | 2.12 | 1.71 |
| 106.0 | 1603.3 | 1366.6 | 15.1 | 12.9 | 2.12 | 1.71 |

The 100 MiB heap drops by 236.7 MiB (14.8%), which is exactly the arithmetic:
705,211 records x (8 elements x 16 B + 7 vector chunks x 32 B) = 86.1 + 150.6
MiB. RSS and peak follow the heap (12.9x and 13.0x).

| kind | heap B before | after | allocations before | after |
|---|---:|---:|---:|---:|
| element | 120 | 104 | 1 | 1 |
| text node | 144 | 112 | 3 | 2 |
| attribute | 208 | 176 | 4 | 3 |

What changed: `theNumAttrs` is a `uint32_t` in the padding after
`XmlNode::theFlags`, and `InternalNode::theNodes` is a `PtrVector`
(`src/store/naive/ptr_vector.h`), 24 B holding up to two pointers inline.
`sizeof(ElementNode)` goes from 96 to 88, chunk 112 to 96; TextNode and
AttributeNode keep their sizes.

Parse time, 100 MiB XML, medians of 3. Runners differ a lot between runs (the
same master code took 3.42 s and 2.05 s), so only runs whose unchanged JSON and
CSV times agree are compared:

| run | code | xml s | json s | csv s |
|---|---|---:|---:|---:|
| 41377872156 | before | 3.42 | 6.61 | 5.06 |
| 41448453817 | after | 3.10 | 6.66 | 5.07 |

XML parsing is 9% faster with JSON and CSV within 1%; the microbenchmarks in
those two runs agree (`x_text1` 104 to 90 ms, `x_attr1` 129 to 118 ms, `x_empty`
53 to 54 ms). A second pair on other runners, 41443565715 (master, xml 2.05 s,
json 5.24 s) and 41442025801 (this change, xml 2.23 s, json 6.22 s), is not
comparable directly; its xml/json ratio falls by 8%.

Rulings:

- **6, further packing: not done.** The next chunk (80 B) needs
  `sizeof(ElementNode)` <= 72, and only the 8 B list pointer can still go. That
  means keeping size and capacity in a heap block, so the 75% of elements that
  have one child would allocate again: +32 B against -16 B. `theTypeName` was
  never stored per element in this build (`EMBEDED_TYPE` is off; types live in
  `XmlTree::theTypesMap`), so that part of the candidate saves nothing.
- **10, done with two slots, not one.** In these files 6 of 8 elements have one
  child and 1 of 8 has two; an element of 88 B has the room in its 96 B chunk.
  Elements with one attribute and one text also fit.
- **9: not done.** It would save an estimated 65 MiB (4.8% of the new heap).
  `getTypedValue()` hands out the stored item today; with a plain string it
  has to allocate one per atomization of an untyped attribute, and about 30
  sites (typed and list values, ID/IDREF checks, update undo) would have to
  branch on the representation. The atomization cost was not measured.

`j_int`, `j_dbl`, `j_str1` and `j_dec` report 2.5 B per item less in these runs
with the same number of blocks, while the JSON file heap does not move. That
pattern fits new items reusing fewer freed start-up chunks larger than they
need; it was not investigated further, and no JSON saving is claimed.

`Item::mem_size()` for XML is unchanged within 8 B; against the smaller heap it
now reads 1.28x instead of 1.09x, so it remains unusable as a meter.

## Limitations

- glibc malloc only; other allocators round differently.
- One record shape per format; documents dominated by long text or by deep
  nesting will weigh content and OrdPaths differently.
- The per-kind costs are measured in isolation; the model's residual
  (-3% to -7%) is where they interact (e.g. vector growth slack).
- Per-kind costs, breakdowns and the ranking above predate "XML node packing";
  only rows 6, 9 and 10 were updated.
- Estimates marked "estimate" are arithmetic on sizeof and chunk sizes, not
  measurements of a modified store.
