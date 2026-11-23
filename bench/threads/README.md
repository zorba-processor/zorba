# Multithreaded throughput

How many queries per second one engine and one store run when N threads query them at
once, and how that scales with N. It measures what a multithreaded engine
(`ZORBA_FOR_ONE_THREAD_ONLY=OFF`) pays for sharing: contended reference counts of shared
items, the store lock, the Xerces mutex. Run at one thread on an ON and an OFF build, it
gives the single-thread cost of the multithreaded build (the "flip cost").

## Files

| File | What it is |
|---|---|
| `mtbench.cpp` | The driver. Public API only; links against libzorba. Compiles one query, then for each thread count runs clones of it on that many threads for a fixed time and counts the completed runs. |
| `run.sh` | Runs the queries below with `mtbench` and writes `runs.jsonl` and `results.md`. |
| `report.xq` | Writes `results.md` from `runs.jsonl`; with a second build's `runs.jsonl`, adds the time relative to that build. |
| `validate.xq`, `orders.xsd` | Validation of 2,000 constructed elements against a schema. |
| `atomics.xq` | 100,000 strings read one by one through the Item API (`mtbench --items`). |

The other queries come from the suites they belong to:

| Name in the report | Query | Input |
|---|---|---|
| `xmark/q01`, `q08`, `q14`, `q19` | `../xmark/` | one XMark document, parsed once and shared by every thread |
| `xmark/.../private` | the same | one copy of the document per thread, parsed before timing |
| `json/count`, `latency`, `filter_project` | `../json-parse/` | each run reads and parses the log |
| `validate` | `validate.xq` | none |
| `api/atomics` | `atomics.xq` | none; each item is read with `Item::getStringValue()` |

## How it measures

`mtbench [--name NAME] [--threads N,N,...] [--seconds S] [-e NAME:=VALUE] [-x NAME=FILE] [--private] [--items] QUERY`

- The query is compiled once, then run once on the main thread; that run's output
  checksum (FNV-1a of the serialized result, or of the string values with `--items`) is
  the one every timed run must reproduce.
- For each thread count N, N threads are released together. Each repeats: clone the
  compiled query, bind its external variables, run it, check the checksum, until `S`
  seconds have passed since the release. A run that starts before the deadline counts
  and is waited for.
- `qps` is the number of runs divided by the time from the release to the end of the
  last run. `scaling` is `qps` divided by the `qps` of the first thread count (1 in
  `run.sh`): N means perfect scaling on N threads.
- A build with `ZORBA_FOR_ONE_THREAD_ONLY` refuses thread counts above 1.
- The process start-up, compilation and document parsing are outside the timed part;
  cloning the query, binding and serializing are inside, as for a server that answers
  each request with a clone.

## Running it

In the CI builder image, from the repository root, with a Release build in `$B` (for
example `cmake --preset multithread` for OFF, `full-release` for ON):

```sh
LIB=$(dirname "$(find "$B" -name libzorba_simplestore.so | head -n 1)")
c++ -std=c++23 -O2 -Wall -Wextra -Werror -I include -I "$B/include" \
  -o /tmp/mtbench bench/threads/mtbench.cpp -L"$LIB" -lzorba_simplestore -Wl,-rpath,"$LIB"
bench/gen_data.sh /tmp/data            # XMark documents (xmark-0.1.xml)
$B/bin/zorba -f -q bench/json-parse/tools/gen.jq -e n:=10000 -e seed:=1 -e out:=/tmp/events.jsonl
timeout 3600 sh bench/threads/run.sh /tmp/mtbench $B/bin/zorba \
  /tmp/data/xmark-0.1.xml /tmp/events.jsonl /tmp/out-off
```

For the single-thread cost of OFF, build `mtbench` against an ON build too and run it at
one thread, then give its runs to the OFF report:

```sh
THREADS=1 timeout 3600 sh bench/threads/run.sh /tmp/mtbench-on $ON/bin/zorba \
  /tmp/data/xmark-0.1.xml /tmp/events.jsonl /tmp/out-on
THREADS=1 BASE=/tmp/out-on/runs.jsonl timeout 3600 sh bench/threads/run.sh \
  /tmp/mtbench $B/bin/zorba /tmp/data/xmark-0.1.xml /tmp/events.jsonl /tmp/out-off1
```

`DURATION` sets the seconds per thread count (default 5), `LIMIT` the limit per query
(default 900 s). Thread counts above the number of CPUs measure oversubscription, not
scaling; the report prints the CPU count (`nproc`).

The suite is driven by its own script, so the `Benchmarks` workflow (`bench.yml`) does not
run it.
