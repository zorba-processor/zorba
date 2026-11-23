# Streaming parse-json of NDJSON logs

How fast `parse-json` reads a log of one JSON object per line, streaming:

```jsoniq
parse-json(file:read-text($log), { "jsoniq-multiple-top-level-items" : true })
```

and where its time and allocations go. Each change that makes it faster adds a row to
[Results](#results), measured the same way.

## Files

| File | What it is |
|---|---|
| `tools/gen.jq` | Writes the synthetic log: `zorba -f -q tools/gen.jq -e n:=200000 -e seed:=1 -e out:=events.jsonl`. The same `n` and `seed` give the same bytes. |
| `tools/shape.jq` | Per-event averages of a log (objects, arrays, keys, strings, numbers, nulls, booleans, bytes). |
| `count.jq` | The parse alone: the number of events. |
| `latency.jq` | Group by service and route: count, average and maximum latency, server errors. |
| `flatten.jq` | Each event as one flat object (JSON lines out). |
| `filter_project.jq` | The server errors of one service, four fields each. |
| `latency_fn.jq` | `latency.jq` with the parse in a `%an:nondeterministic` function, and the status of the slowest request of each group (by position). |
| `windows.jq` | Requests, server errors and the slowest request per minute, with a tumbling window. |
| `clean_users.jq` | Requests per service of the users who never got a server error: a FLWOR gives the events of a group by to an enclosing group by. |
| `unmatched.jq` | Per service, the server errors of users with no successful request: a general `=` against the distinct user ids of a second parse. |
| `run.sh` | Best of 3 runs of the queries: wall time, peak RSS, MB/s, SHA-256 of the output. |
| `tools/profile.sh` | Samples per part (perf + `classify.awk`) and allocations per event (`malloc_count.c`) of one query. |
| `tools/classify.awk` | Charges each `perf script` sample to a part of the parse or of the query. |
| `tools/malloc_count.c` | LD_PRELOAD library that counts malloc, calloc and realloc calls and the bytes requested. |

The queries take the log as `-e log:=FILE`.

## The synthetic log

The first profile was taken on a private log; its content stays private, and only its
shape per event was kept. `gen.jq` writes events of that shape (application log
records with a trace, an HTTP request, a client with a location, metrics and a user;
optional cache, tags and sampled fields). `tools/shape.jq` on 20,000 events of seed 1:

| per event | private log | synthetic |
|---|---:|---:|
| bytes | ~1,065 | 1,064.6 |
| objects | 7.5 | 7.5 |
| arrays | 0.24 | 0.24 |
| keys | 36 | 36.3 |
| strings | 21 | 20.9 |
| numbers (non-integer) | 6.8 (4.0) | 6.8 (4.0) |
| nulls | 2.1 | 2.07 |
| booleans | 0.26 | 0.26 |

200,000 events of seed 1 are 212,898,225 bytes (SHA-256 `56d4a3ad79c6ae50...`).
Writing them takes about 4 minutes.

## Running it

In the CI builder image, from the repository root, with a `full-release` build:

```sh
c++ -std=c++23 -O2 -o /tmp/dirbench bench/directory/dirbench.cpp
zorba -f -q bench/json-parse/tools/gen.jq -e n:=200000 -e seed:=1 -e out:=/tmp/events.jsonl
timeout 3600 sh bench/json-parse/run.sh build/full-release/bin/zorba /tmp/dirbench /tmp/events.jsonl /tmp/out
```

`JQ=/path/to/jq` also times jq counting the events. Two commits are compared on
GitHub Actions with the `Benchmarks` workflow (see [../README.md](../README.md)); the
queries are the `*.jq` files of this directory, so the suite runs as it is:

```sh
gh workflow run bench.yml -R zorba-processor/zorba --ref master \
  -f suite=json-parse -f ref_a=<base sha> -f ref_b=<head sha> -f runs=3 \
  -f setup='$ZORBA -f -q $SUITE/tools/gen.jq -e n:=200000 -e seed:=1 -e out:=$DATA/events.jsonl' \
  -f args='-e log:=$DATA/events.jsonl'
```

## How the profile is taken

1. **Build** with frame pointers and symbols, otherwise as `full-release`:
   `cmake --preset full-release -DCMAKE_CXX_FLAGS_RELEASE="-O3 -DNDEBUG -g -fno-omit-frame-pointer" -DCMAKE_C_FLAGS_RELEASE="-O3 -DNDEBUG -g -fno-omit-frame-pointer"`.
2. **Sample** one query at a time with `perf record -g --call-graph fp`. Frame pointers
   give whole call stacks at low cost; DWARF unwinding is not needed.
3. **Classify** the samples with `classify.awk`: each sample goes to the innermost
   frame that names a part (`read`, `tokenize`, `grammar`, `loader`, `containers`,
   `keys`, `numbers`, `strings`, `free`, or a part of the query: `q_*`, `iterators`).
   Generic helpers (string and reference-count code, hash tables, malloc/free, the C++
   library) are charged to their first caller that names a part, so that malloc in
   the lexer counts as tokenizing. A part's seconds are its share times the best wall
   time of the query.
4. **Count allocations** with `malloc_count.so` preloaded, over the log and over an
   empty file; the difference divided by the events is the allocations and bytes per
   event, net of startup.

`tools/profile.sh ZORBA QUERY LOG OUTDIR` does steps 2 to 4 (perf must be allowed:
`kernel.perf_event_paranoid` at most 1, or a container with `--cap-add SYS_ADMIN`).

### Prototypes before the changes

Before PR 1, each candidate change was tried as a throwaway patch behind an environment
variable, in one binary, against the unpatched build (interleaved, best of 3): reading
characters through the stream buffer instead of `istream::get()/peek()`, releasing a
shared string on `clear()` instead of reallocating it, reserving the pairs of an
object, building numbers as `xs:double`, and the loader's `check_only_` mode as the
ceiling of parse-time projection. Their gains are lower bounds: a toggle tested at run
time keeps the branch and the slower code in the binary, and the patches did only the
minimum. The profile and the ranked list of changes that came out of them are the
basis of the three pull requests (PR 1: items 1, 2, 3, 7; PR 2: 4, 5; PR 3: 6).

## Results

All rows are measured on the same machine and container class: a Mac Pro (Intel Xeon
W-3245, 16 cores), Docker Desktop's Linux VM with 16 CPUs and 31 GiB, a container of
the builder image (`docker/builder-latest`, built locally), the build of step 1 of the
profile. The log is 200,000 events of seed 1 (212.9 MB). Unlike the other suites of
`bench/`, these numbers come from that machine, not from GitHub Actions; the host ran
other builds at the same time, so base and head were measured in alternating rounds,
and each time is the best of 3 runs of every round. MB/s is 10^6 bytes of the log per
second. Peak RSS is 35 MiB for every query but `latency` (885 MiB before PR 1, 866
MiB after, 148 MiB from PR 4), and every query writes the same output on every row.

### Parse (`count.jq`)

Seconds per part are the part's share of the perf samples times the best wall time.
`other` is the query's iterators and startup.

| Build | Wall s | MB/s | tokenize | grammar | loader | containers | keys | numbers | strings | free | read | other | allocations / event | bytes / event |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| master `ab93bb29c6` | 5.72 | 37.2 | 3.02 | 1.01 | 0.55 | 0.50 | 0.17 | 0.08 | 0.02 | 0.22 | 0.07 | 0.09 | 415 | 12,744 |
| PR 1 | 3.88 | 54.9 | 1.65 | 0.69 | 0.53 | 0.45 | 0.07 | 0.08 | 0.02 | 0.23 | 0.07 | 0.10 | 224 | 7,696 |
| PR 2 | 2.38 | 89.5 | 0.51 | 0.36 | 0.47 | 0.48 | 0.09 | 0.08 | 0.04 | 0.19 | 0.08 | 0.09 | 128 | 5,458 |
| PR 3 | 1.66 | 128.3 | 0.44 | 0.36 | 0.46 | 0.20 | 0.06 | 0.03 | 0.00 | 0.00 | 0.07 | 0.05 | 43 | 1,839 |
| jq 1.8.2 (same container) | 2.81 | 75.8 | | | | | | | | | | | | |
| jq 1.7.1 (macOS on the host, same machine) | 3.68 | 57.9 | | | | | | | | | | | | |

jq counts with `jq -n 'reduce inputs as $e (0; . + 1)'`; jq 1.7.1 is not packaged for
the builder image, so it ran natively on the host. jq 1.8.2 took 2.78 s again in the
rounds of the PR 2 row.

### All four queries (best wall time, s)

| Build | count | latency | flatten | filter_project |
|---|---:|---:|---:|---:|
| master `ab93bb29c6` | 5.72 | 8.07 | 8.85 | 6.00 |
| PR 1 | 3.88 (-32%) | 5.93 (-27%) | 6.90 (-22%) | 4.07 (-32%) |
| PR 2 | 2.38 (-58%) | 4.40 (-45%) | 5.50 (-38%) | 2.64 (-56%) |
| PR 3 | 1.66 (-71%) | 4.33 (-46%) | 4.99 (-44%) | 2.09 (-65%) |
| PR 4 | 1.75 (-69%) | 2.93 (-64%) | 3.46 (-61%) | 2.09 (-65%) |

Percentages are against master `ab93bb29c6`. The PR 2 row was measured with the host
otherwise nearly idle, alternating with the PR 1 head (`06261a9290`), which took 3.79,
5.89, 6.94 and 4.05 s in those rounds: PR 2 alone is -37%, -25%, -21% and -35%.

From PR 3 on, at `-O1` and `-O2` (the queries run at the default, `-O1`), a parse
builds only what the query reads of each value when that can be proven: `count.jq`
builds no content of the events (hence no strings and nothing to free in its row),
`flatten.jq` and `filter_project.jq` build the fields they look up. `latency.jq`
still builds every event: after its `group by` it counts the events and filters
them with a predicate, which the analysis does not follow. The PR 3 row alternated
with the PR 2 head (`8611ea3260`), which took 2.33, 4.28, 5.40 and 2.58 s in those
rounds: PR 3 alone is -29%, +1% (`latency`, not projected: noise), -8% and -19%.

From PR 4 on, the analysis follows the `for` variable through `group by`, where it
is bound to the events of each group, and accepts counting them (`count`, `empty`,
`exists`), predicates whose `$$` is only looked up with literal keys, and lookups of
literal keys in the group (also in aggregates, and as `$e("key")`). `latency.jq` now
builds of each event only `service`, `http.route`, `http.duration_ms` and
`http.status`, and its peak RSS drops from 866 to 148 MiB. The PR 4 row alternated
with the PR 3 head (`d879336fc0`), which took 1.77, 4.60, 3.44 and 2.10 s in those
rounds: PR 4 alone is -1%, -36% (`latency`), +1% and 0% (the other three are not
changed: noise). `flatten` took 3.44 s on the PR 3 head in these rounds and 4.99 s
in those of the PR 3 row: rows compare only within their own rounds.

### Function calls, windows and enclosing uses (PR 5)

`latency_fn`, `windows` and `clean_users` were added with PR 5. Best wall time (s) and
peak RSS (MiB) of two alternating rounds, master `4fc51940ef` against the PR 5 head:

| query | master s | PR 5 s | master MiB | PR 5 MiB |
|---|---:|---:|---:|---:|
| count | 1.66 | 1.67 | 35 | 35 |
| latency | 2.72 | 2.71 | 148 | 148 |
| flatten | 3.26 | 3.26 | 35 | 35 |
| filter_project | 1.99 | 1.99 | 35 | 35 |
| latency_fn | 4.99 | 3.21 (-36%) | 869 | 150 |
| windows | 3.02 | 2.10 (-30%) | 35 | 35 |
| clean_users | 4.75 | 3.00 (-37%) | 921 | 201 |

On master, `latency_fn` builds every event: its parse is in a function that calls
`file:read-text`, a user function itself, and only functions that called no user
function were inlined. From PR 5 on, a function whose optimized body calls none is
inlined too, and the analysis accepts window clauses over a parse, values taken by
position, and values that a FLWOR gives to an enclosing lookup or `for` clause
(`clean_users`). The four earlier queries are not changed (noise); every query writes
the same output on both builds.

### A general `=` against a `let` variable (`unmatched.jq`)

`unmatched.jq` parses the log twice: once for the distinct ids of the users with a
successful request, bound by a `let`, and once for the server errors, each compared
with that variable by `not($e.user.id = $succeeded)`. Until it was hashed, the
comparison went over every item of the variable for each error.

| Build | Wall s | MB/s | Peak RSS MiB |
|---|---:|---:|---:|
| master `5fe30a45e6` | 118.86 | 1.8 | 150 |
| hashed `let` operand | 6.25 (-95%) | 34.1 | 153 |

Measured in alternating rounds with that master, in a container limited to 6 CPUs
with the host busy with other builds: master took 118.86, 174.85 and 157.07 s, the
head 6.66, 6.32 and 6.25 s. The output is the same.

### One allocation per object

An object built from a known number of pairs (the loader's objects among them) stores
its pairs in its own allocation instead of a separate vector. Master `25b1433435`
against the head, in a container limited to 4 CPUs with the host busy with other
builds; allocations from `tools/profile.sh`, times the best of 5 runs (6 for
`flatten`) alternating master and head run by run, peak RSS from `run.sh` (two
rounds; equal in both).

| query | level | master allocations / event | head | master bytes / event | head |
|---|---|---:|---:|---:|---:|
| count | -O0 | 127.7 | 120.2 | 5,458 | 5,458 |
| count | -O1 | 42.7 | 42.7 | 1,839 | 1,839 |
| latency | -O1 | 65.7 | 63.7 | 2,888 | 2,888 |

At `-O0` the parse builds all 7.5 objects of an event, and each now takes one
allocation instead of two; the bytes requested are the same (the object and its pairs,
now in one block). At `-O1`, `count.jq` builds no content and `latency.jq` builds two
objects per event.

| query | level | master s | head s | master MiB | head MiB |
|---|---|---:|---:|---:|---:|
| count | -O0 | 2.56 | 2.55 | 36 | 36 |
| latency | -O0 | 5.23 | 5.12 (-2%) | 867 | 844 |
| flatten | -O0 | 4.60 | 4.45 (-3%) | 36 | 36 |
| clean_users | -O0 | 5.39 | 5.38 | 921 | 898 |
| latency | -O1 | 3.01 | 2.88 (-4%) | 148 | 142 |

Every query writes the same output on both builds (`unmatched.jq` at `-O0` takes over
the 600 s limit on both).
