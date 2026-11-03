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

The hit tracker is 4096 tags of 8 bytes, 32 KiB, allocated once.

## Long sessions

Run 41622009308 (commit a9e65c56c), `dirbench soak` through `run_cache.sh`: one session per row and number of calls, `--no-scan`, `--timeout 0`, 0 failed calls in all of them. Peak RSS is `VmHWM`. Admissions and evictions are those of the longest session.

- `hits`: a scan of all the files.
- `turnover`: one file per query, each read twice, more files than slots: every second call admits and evicts.
- `rewrite`, `touch`, `delete`: a scan, and before it one file is rewritten with another size, gets a new modification time, or is deleted and written again.
- `refused`: a scan of files that do not parse.

| scenario | format | files x KB | KB:files | calls | peak RSS MiB | bytes retained | admissions | evictions |
|---|---|---|---|---|---|---|---:|---:|
| hits | xml | 10x2 | 16:10 | 100 / 1000 / 10000 | 37.9 / 38.0 / 37.9 | 600190 / 600200 / 600210 | 10 | 0 |
| hits | json | 10x2 | 16:10 | 100 / 1000 / 10000 | 37.9 / 37.4 / 37.4 | 346020 / 346020 / 346020 | 10 | 0 |
| hits | csv | 10x2 | 16:10 | 100 / 1000 / 10000 | 38.0 / 37.8 / 38.0 | 1137420 / 1137420 / 1137420 | 10 | 0 |
| hits | html | 10x2 | 16:10 | 100 / 1000 / 10000 | 38.0 / 37.9 / 38.0 | 440850 / 440860 / 440870 | 10 | 0 |
| turnover | xml | 15x2 | 16:10 | 100 / 1000 / 10000 | 37.9 / 37.7 / 37.9 | 591390 / 591410 / 591430 | 5000 | 4990 |
| turnover | json | 15x2 | 16:10 | 100 / 1000 / 10000 | 37.1 / 37.3 / 37.4 | 339725 / 339725 / 339725 | 5000 | 4990 |
| turnover | csv | 15x2 | 16:10 | 100 / 1000 / 10000 | 37.8 / 37.8 / 37.8 | 1075260 / 1075260 / 1075260 | 5000 | 4990 |
| turnover | html | 15x2 | 16:10 | 100 / 1000 / 10000 | 38.2 / 38.0 / 38.0 | 433900 / 433920 / 433940 | 5000 | 4990 |
| rewrite | xml | 10x2 | 16:10 | 100 / 1000 / 10000 | 38.2 / 38.3 / 37.9 | 421944 / 421962 / 421980 | 10007 | 0 |
| rewrite | json | 10x2 | 16:10 | 100 / 1000 / 10000 | 37.1 / 37.9 / 37.6 | 243930 / 243930 / 209328 | 10006 | 0 |
| rewrite | csv | 10x2 | 16:10 | 100 / 1000 / 10000 | 37.8 / 38.0 / 38.1 | 796350 / 796350 / 796350 | 10007 | 0 |
| rewrite | html | 10x2 | 16:10 | 100 / 1000 / 10000 | 37.9 / 37.8 / 38.4 | 267680 / 311788 / 267712 | 10006 | 0 |
| touch | xml | 10x2 | 16:10 | 100 / 1000 / 10000 | 38.0 / 38.2 / 38.1 | 480176 / 540216 / 540234 | 10007 | 0 |
| touch | json | 10x2 | 16:10 | 100 / 1000 / 10000 | 37.8 / 37.4 / 37.6 | 276816 / 276816 / 276816 | 10006 | 0 |
| touch | csv | 10x2 | 16:10 | 100 / 1000 / 10000 | 38.0 / 38.3 / 38.0 | 1023678 / 1023678 / 1023678 | 10007 | 0 |
| touch | html | 10x2 | 16:10 | 100 / 1000 / 10000 | 38.2 / 38.2 / 38.4 | 396792 / 396810 / 396828 | 10007 | 0 |
| delete | xml | 10x2 | 16:10 | 100 / 1000 / 10000 | 38.0 / 38.4 / 37.8 | 540207 / 480200 / 480216 | 10006 | 0 |
| delete | json | 10x2 | 16:10 | 100 / 1000 / 10000 | 37.3 / 37.8 / 37.4 | 311418 / 311418 / 276816 | 10006 | 0 |
| delete | csv | 10x2 | 16:10 | 100 / 1000 / 10000 | 38.0 / 38.0 / 38.0 | 1023678 / 1023678 / 1023678 | 10007 | 0 |
| delete | html | 10x2 | 16:10 | 100 / 1000 / 10000 | 38.1 / 38.1 / 38.0 | 396801 / 396819 / 352744 | 10006 | 0 |
| refused | xml | 10x2 | 16:10 | 100 / 1000 / 10000 | 38.9 / 39.0 / 38.6 | 0 / 0 / 0 | 0 | 0 |
| refused | json | 10x2 | 16:10 | 100 / 1000 / 10000 | 38.7 / 38.7 / 38.6 | 0 / 0 / 0 | 0 | 0 |
| hits | xml | 10x2 | 0:0 | 100 / 1000 / 10000 | 36.6 / 36.8 / 36.7 | - / - / - | - | - |
| hits | json | 10x2 | 0:0 | 100 / 1000 / 10000 | 36.7 / 36.3 / 36.5 | - / - / - | - | - |
| hits | csv | 10x2 | 0:0 | 100 / 1000 / 10000 | 36.8 / 36.5 / 36.6 | - / - / - | - | - |
| hits | html | 10x2 | 0:0 | 100 / 1000 / 10000 | 36.9 / 37.0 / 36.6 | - / - / - | - | - |
| turnover | xml | 15x2 | 0:0 | 100 / 1000 / 10000 | 36.6 / 36.4 / 36.4 | - / - / - | - | - |
| turnover | json | 15x2 | 0:0 | 100 / 1000 / 10000 | 36.0 / 36.2 / 36.2 | - / - / - | - | - |
| turnover | csv | 15x2 | 0:0 | 100 / 1000 / 10000 | 36.3 / 36.0 / 36.0 | - / - / - | - | - |
| turnover | html | 15x2 | 0:0 | 100 / 1000 / 10000 | 36.6 / 36.7 / 36.5 | - / - / - | - | - |
| turnover | xml | 6x3072 | 8192:2 | 100 / 1000 | 292.0 / 292.8 | 182191858 / 182191862 | 500 | 498 |
| turnover | json | 6x3072 | 8192:2 | 100 / 1000 | 152.7 / 152.6 | 103460820 / 103460820 | 500 | 498 |
| turnover | csv | 6x3072 | 8192:2 | 100 / 1000 | 424.9 / 424.9 | 349175964 / 349175964 | 500 | 498 |
| turnover | html | 6x3072 | 8192:2 | 100 / 1000 | 226.5 / 232.0 | 131598442 / 131598446 | 500 | 498 |
| turnover | xml | 6x3072 | 0:0 | 100 / 1000 | 147.4 / 147.4 | - / - | - | - |
| turnover | json | 6x3072 | 0:0 | 100 / 1000 | 85.5 / 85.7 | - / - | - | - |
| turnover | csv | 6x3072 | 0:0 | 100 / 1000 | 205.8 / 205.8 | - / - | - | - |
| turnover | html | 6x3072 | 0:0 | 100 / 1000 | 119.5 / 119.1 | - / - | - | - |

Peak RSS does not grow with the calls on files of 2 KB: the largest difference between 100 and 10000 calls is 0.5 MiB, in either direction, with or without the cache. On files of 3 MiB, measured to 1000 calls only, the largest difference between 100 and 1000 calls is 5.5 MiB, HTML (226.5 to 232.0); the others are within 0.8 MiB. In `turnover` at 10000 calls, 5000 records were admitted and 4990 released with RSS where it was at 100 calls.

`bytes_retained` stays flat. Two things move it, neither growing with use:

- An XML or HTML record holds a generated document URI with the number of its tree, so a record built later takes a byte or two more (`hits`, XML: 600190, 600200, 600210, ten records).
- In `rewrite`, `touch` and `delete`, the file changed before the last scan may not be held when the session ends, and the rewritten files alternate between two sizes. The value depends on where the session stops, not on how long it ran.

Six files of about 3 MiB with two slots hold two records, as the table shows; the RSS over no cache is what they account for.

ASan with leak detection covers the same paths in `bin/mcp/cache_turnover`, `cache_invalidate_loop`, `cache_refused_loop` and `cache_session_binary`, for all formats.

## One large file, kept

Same run. One file of 30 MiB, `--cache-max-file-kb` 61440, five calls of `large.xq` (node or row count, sum, first record). The last response has the same bytes as without a cache in all formats.

| format | peak RSS MiB | hits | misses | admissions | bytes retained |
|---|---:|---:|---:|---:|---:|
| XML | 939 | 3 | 2 | 1 | 669,761,129 |
| JSON | 364 | 3 | 2 | 1 | 327,044,980 |
| CSV | 593 | 3 | 2 | 1 | 564,043,296 |
| HTML | 1173 | 3 | 2 | 1 | 796,544,440 |

## What a hit costs now

A hit returns a copy of the record, so that a query sees what it would see reading the file. Later calls on ten files read repeatedly, against no cache, 5 runs per setting, same run:

| tree | scan | aggregate |
|---|---:|---:|
| 10x2KB-xml | -15.0% | -12.9% |
| 10x16KB-xml | -33.2% | -30.4% |
| 10x16KB-json | -40.7% | -37.7% |
| 10x16KB-csv | -41.5% | -37.6% |

The sections above this one were measured before the copy. Nothing else in the matrix was measured again.

## The hit tracker and collisions

Before run 41622009308 the tracker had one tag per slot. Two files in a slot replace each other at each read and never enter. With tags that behave as uniform, a file is alone in its slot with probability (1 - 1/4096)^(n-1): 783.5 of 1000 expected, against 754 to 804 measured (runs 41547644611, 41567671862). For ten files, one pair collides in 1.09% of trees; tree 10x64KB-csv-fields in run 41497878541 admitted 8 of 10 in all its sessions, while the same shape admitted 10 in run 41494352367. At 100 files 70% of trees lose at least one pair.

It is now 4096 tags in sets of 8, the same 32 KiB, and a tag finding its set full replaces one chosen at random. Tags of files read once stay until replaced. A choice fixed by the tag, tried first, had two files of a full set replace each other forever.

A simulation with random tags, 50 scans, 200 trees (60 for the largest), files that never entered, on average and in how many trees:

| files read once, then files scanned | choice fixed by the tag | chosen at random |
|---|---:|---:|
| 0, 1000 | 0, 0% | 0, 0% |
| 0, 4096 | 75.7, 98% | 0, 0% |
| 0, 10000 | 7160, 100% | 3030, 100% |
| 3000, 100 | 3.0, 78.5% | 0, 0% |
| 10000, 300 | 15.6, 99.5% | 0, 0% |
| 20000, 1000 | 28.8, 100% | 0, 0% |

At random, in 10000 then 300, 24 files on average are still out after the second scan and none after the fourth. With far more files than tags, as in 0 then 10000, many stay out. On Actions, `bin/mcp/cache_slack_many` and `cache_full_tracker` cover 100 and 300 files. The "778-804 of 1000" above predates this.
