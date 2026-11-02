# Store memory: results

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
| 1 | Share key items across the objects of one parse (as csv:parse does) | measured: JSON pair 182 B vs CSV pair 86 B = 96 B/pair | - | 673 MiB (32%) | already done |
| 2 | Per-document arena (bump) allocation of nodes and items: no per-block header or rounding | measured headers only (8 B x blocks); rounding not measured | >= 194 MiB (12%) | >= 289 MiB (14%) | >= 186 MiB (11%) |
| 3 | Small-object JSON keys without `unordered_map` (linear search over the pair vector for small objects) | estimate: map node 32 B (chunk 48) + bucket array per key | - | ~340 MiB (16%) | ~410 MiB (25%) |
| 4 | Immediate (tagged) or cached small integers and doubles instead of a 48 B item | measured: integer 58.5 B vs boolean 8 B (shared) = 50.5 B | - | 203 MiB (10%) + doubles 25 MiB | 123 MiB (7%) |
| 5 | Text stored inline in TextNode (small-string), no separate rep | estimate: rep chunk >= 32 B per text node | 151 MiB (9%) | - | - |
| 6 | Pack ElementNode (96 B) to 80 B or 64 B (32-bit counts, no `theTypeName` for untyped, smaller child vector header) | estimate: chunk 112 -> 96 / 80 | 86-172 MiB (5-11%) | - | - |
| 7 | Decimals without MAPM heap for values that fit (e.g. int64 mantissa + scale) | measured: 144 B per decimal outside operator new | - | 72 MiB (3%) | 176 MiB (11%) |
| 8 | Short strings inline in StringItem (no rep) | estimate: rep chunk >= 32 B per string | - | 80 MiB (4%) | 117 MiB (7%) |
| 9 | Attribute value as a plain string instead of an UntypedAtomicItem | estimate: 48 B chunk per attribute | 65 MiB (4%) | - | - |
| 10 | Child vector inline for elements with one child | estimate: vector chunk >= 32 B per element with children | not measured (needs a count of single-child elements) | - | - |

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

## Limitations

- glibc malloc only; other allocators round differently.
- One record shape per format; documents dominated by long text or by deep
  nesting will weigh content and OrdPaths differently.
- The per-kind costs are measured in isolation; the model's residual
  (-3% to -7%) is where they interact (e.g. vector growth slack).
- Estimates marked "estimate" are arithmetic on sizeof and chunk sizes, not
  measurements of a modified store.
