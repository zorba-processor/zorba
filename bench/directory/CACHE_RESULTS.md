# zorba-mcp document cache: measurements

`bench/directory/run_cache.sh` (`directory-scaling.yml`, suite `cache`) on one GitHub runner per run: one session per setting, with the settings alternating in rotating order, `--no-scan`, `--timeout 0`. Values are the median over the runs. "Later" is the median of calls 2..6. Percentages compare with no cache in the same run. A setting is `--cache-max-file-kb:--cache-max-files`.

## The default: 16:10

Run 41567671862 (5 runs per setting; commit 60896f23b) and, for trees whose files are over 16 KB, run 41547644611 (5 runs; setting 32:10, in which those files get one `stat`, the same path as at 16:10; commit f8c1da037).

| tree | query | call 1 ms, off | call 1 vs off | later ms, off | later vs off | peak RSS MiB, off / 16:10 |
|---|---|---:|---:|---:|---:|---:|
| 1000x2KB-xml | scan | 384.3 | +2.1% | 378.8 | +2.3% | 38 / 38 |
| 1000x2KB-xml | aggregate | 404.3 | +2.2% | 399.6 | +2.3% | 37 / 39 |
| 1000x16KB-xml | scan | 1036.5 | +1.2% | 1033.8 | +1.8% | 38 / 42 |
| 1000x16KB-xml | aggregate | 1139.2 | +0.6% | 1135.7 | +1.1% | 38 / 42 |
| 1000x16KB-json | aggregate | 1160.0 | +1.1% | 1162.2 | +0.9% | 38 / 40 |
| 1000x16KB-csv | scan | 1188.6 | +1.0% | 1188.9 | +0.5% | 37 / 40 |
| 1000x16KB, elements with an attribute | scan | 1429.4 | +0.5% | 1426.4 | +0.8% | 38 / 43 |
| 1000x16KB, CSV of one short column | scan | 3752.3 | -0.3% | 3722.0 | +0.2% | 40 / 50 |

Over 16 KB, run 41547644611:

| tree | query | call 1 vs off | later vs off | peak RSS MiB, off / 32:10 |
|---|---|---:|---:|---:|
| 100x200KB-xml | scan, aggregate | -0.2%, +0.2% | +0.2%, +0.1% | 48 / 48 |
| 100x200KB-json | scan, aggregate | +0.2%, +0.1% | +0.1%, +0.1% | 41 / 41 |
| 100x200KB-csv | scan, aggregate | -0.3%, +0.0% | -0.7%, +0.0% | 43 / 43 |
| 1000x200KB-xml | scan, aggregate | +0.0%, +0.1% | +0.0%, +0.2% | 48 / 48 |
| 1000x200KB-json | scan, aggregate | +0.5%, +0.2% | +0.3%, +0.4% | 41 / 41 |
| 1000x200KB-csv | scan, aggregate | +0.0%, +0.1% | -0.0%, -0.0% | 44 / 44 |

At 32:10, on trees of 2 KB files (the same path as 16:10), run 41547644611:

| tree | query | call 1 vs off | later vs off |
|---|---|---:|---:|
| 10000x2KB-xml | scan, aggregate | +1.7%, +1.3% | +2.1%, +1.8% |
| 10000x2KB-json | scan, aggregate | +1.3%, +1.6% | +1.4%, +1.6% |
| 10000x2KB-csv | scan, aggregate | +1.2%, +1.2% | +0.8%, +0.8% |
| 1000x2KB-json | scan, aggregate | +1.8%, +0.9% | +1.9%, +0.7% |
| 1000x2KB-csv | scan, aggregate | +1.4%, +0.8% | +0.5%, +0.8% |

The worst case at 16:10 is +2.3%, within the bar of 5%.

## Why not 64 or 32

Later calls, compared with no cache, on the same runner:

| setting | tree | scan | aggregate | run |
|---|---|---:|---:|---|
| 64:10 | 1000x64KB-xml | +9.8% | +8.1% | 41529522453 (7 runs) |
| 64:10 | 1000x64KB-xml | +9.0% | +8.4% | 41509156632 (7 runs) |
| 64:10 | 1000x64KB-json | +2.6% | +5.0% | 41529522453 |
| 32:10 | 1000x32KB-xml | +2.6% | +2.8% | 41547644611 |
| 32:10 | 1000x32KB, elements with an attribute | +7.1% | | 41547644611 |
| 32:10 | 1000x32KB, elements with an attribute | +7.1% | | 41566569728 (7 runs) |
| 16:10 | 1000x16KB, elements with an attribute | +0.8% | | 41566569728, 41567671862 |
| 256:10 | 100x200KB-xml | +17.7% | +28.2% | 41497878541 (3 runs) |
| 256:10 | 1000x200KB-xml | +15.6% | +19.1% | 41501202764 (3 runs) |

The cost grows with the size of the XML documents held: over many files, a few large XML documents in memory slow the parsing of the others by more than their hits save. Why is not established. Runs 41497878541 to 41566569728 predate the XML node packing (#145), which landed in the commit of run 41567671862; 32 and 64 were not measured again after it.

N = 16, 64, 256 at 10 files on the other trees, before #145: runs 41497878541 and 41501202764. Between runs, sessions that take the same path differed by up to 11% on trees of 200 KB files in 41501202764 (1000x200KB-csv aggregate: 64:10 +11.2%, 64:1000 +0.4%, zero admissions in both); 41547644611 has those same trees within 0.7%.

## Where the rule applies

Ten files read repeatedly, run 41567671862:

| tree | query | later ms, off | later ms, 16:10 | vs off |
|---|---|---:|---:|---:|
| 10x2KB | scan | 13.8 | 11.6 | -15.9% |
| 10x2KB | aggregate | 14.7 | 12.5 | -15.0% |
| 10x16KB | scan | 20.6 | 11.7 | -43.2% |
| 10x16KB | aggregate | 22.5 | 13.5 | -40.0% |

What remains is the listing and the rest of the call. Call 1 is within +1.5%. The agent benchmark spends 2.3% of its session time in queries, so no benefit is claimed for it.

With more files allowed, when the cache holds much of the tree, run 41547644611, later calls: 1000x2KB at 32:1000 -44% to -52% (778-804 of 1000 documents held, peak RSS 51-71 MiB against 38), 10000x2KB at 32:1000 -0.2% to -5.0%. In run 41567671862, 1000x16KB-csv at 16:1000 -57% to -66.5% with peak RSS 204-205 MiB against 37-38.

## Memory bound

`--cache-max-files` times `--cache-max-file-kb` times 128: a record whose `Item::mem_size` is over 128 times its file size (at least 1 KiB) is not kept. At the defaults, 10 x 16 KiB x 128 = 20.5 MiB accounted. `Item::mem_size` counts more than RSS shows: about twice for CSV.

Worst shapes (`gen_worst.xq`), 10 files, `scan`:

| shape | 10 x 16 KB, bytes retained | peak RSS MiB, off / 16:10 | 10 x 64 KB, bytes retained | peak RSS MiB, off / 64:10 |
|---|---:|---:|---:|---:|
| XML, empty elements | 7,787,450 | 38 / 42 | 31,134,810 | 42 / 58 |
| XML, elements with an attribute | 7,794,140 | 38 / 42 | 31,754,080 | 42 / 59 |
| JSON, empty arrays | 3,937,140 | 38 / 43 | 15,733,620 | 42 / 57 |
| JSON, objects of one member | 3,812,640 | 37 / 40 | 15,240,480 | 40 / 47 |
| CSV, one short column | 15,240,460 | 39 / 48 | 60,951,820 | 48 / 79 |
| CSV, four empty columns | 19,328,880 | 39 / 45 | 77,328,240 | 44 / 66 |
| HTML, `<p>` only | 10,488,550 | 39 / 44 | 41,946,070 | 44 / 65 |

The 16 KB columns are run 41567671862; the 64 KB columns are run 41494352367, before the limit of 128 times and before #145.

A small file can expand far beyond that without the limit: an XML file of 4.8 KB with one internal entity of 1000 elements referenced 240 times, which libxml2 accepts. Ten of them, without the limit: 457,780,370 bytes retained and peak RSS 340 MiB against 119 without a cache (run 41496831344). With the limit they are not kept: 0 bytes retained, peak RSS 109 / 108 MiB (run 41567671862). 1000 references are refused by libxml2 and yield an error record.

The hit tracker is 4096 slots of 8 bytes, 32 KiB, allocated once.
