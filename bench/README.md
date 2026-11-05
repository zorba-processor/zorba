# Benchmarks

Each directory holds one benchmark suite. Its `RESULTS.md` records measurements made
on GitHub Actions, with the run id. Nothing here is measured on a developer machine.

## Comparing two commits: the `Benchmarks` workflow with `suite`

`.github/workflows/bench.yml` builds two commits in Release (`full-release`, the CI
builder image, ccache) in one job on one runner and runs the queries of a suite on
both. Because the builds share the runner and the runs alternate, runner noise hits both
sides alike. It only measures: it never commits or pushes. Dispatch it from the default
branch (a `workflow_dispatch` workflow must be on it):

```sh
gh workflow run bench.yml -R zorba-processor/zorba --ref master \
  -f suite=dynamic-calls -f base=<base sha> -f head=<head sha> -f repeat=5
gh run watch -R zorba-processor/zorba            # then open the run's summary
```

Inputs:

| Input | Meaning | Default |
|---|---|---|
| `suite` | A directory of `bench/`, e.g. `dynamic-calls`. Empty: the XMark + JSONiq benchmark (`ref_a`, `ref_b`, `runs`, below). | empty |
| `base` | The commit to compare against (branch, tag or SHA). Required with `suite`. | |
| `head` | The commit under test. | the ref the workflow runs on |
| `filter` | Space or comma separated query names (file name without extension; shell patterns such as `pure_*` work). | all |
| `repeat` | Timed runs per query and build, after one warm-up run. | 5 |
| `timeout` | Seconds per run. A run over it is stopped and has status `timeout`. | 300 |
| `args` | Extra zorba arguments for every query, split as the shell does, e.g. `-e n:=100000`. `$DATA` is the data directory. | none |
| `setup` | Shell command that prepares data in `$DATA` before measuring. `$ZORBA` is the head build, `$SUITE` the suite directory. | none |
| `suite_from` | Which commit provides the suite directory: `head`, `base` or `workflow`. Both builds always run the same query files. | `head` |

The scripts (`compare.sh`, `compare_table.xq`, `directory/dirbench.cpp`) come from the
ref the workflow runs on. The job stops starting queries after 3.5 hours (the rest are
reported as `not-run`) and is limited to 345 minutes.

### What a suite is

The queries are the `*.xq` and `*.jq` files directly in the suite directory; each runs as
`zorba -f -q FILE ARGS`, a fresh process per run. A query that needs input takes it from
`args`/`setup`. For example the window queries over 100000 synthetic log records:

```sh
gh workflow run bench.yml -R zorba-processor/zorba --ref master \
  -f suite=windows -f base=<sha> -f head=<sha> \
  -f filter='baseline moving_points moving_minutes duration sessions' \
  -f setup='$ZORBA --omit-xml-declaration --serialize-text -f -q $SUITE/gen.jq -e n:=100000 -e form:=lines > $DATA/log.jsonl' \
  -f args='-e source:=lines:$DATA/log.jsonl'
```

Suites that are driven by their own script and server (`scale`, `directory`, `store`,
`limits`, `agents`) are not run by this workflow; `directory-scaling.yml` and
`agent-benchmark.yml` run them.

### How a comparison runs

For each query: one warm-up run of each build (not in the statistics), then `repeat` timed
runs of each. The builds alternate, and the build that goes first alternates from run to
run. Wall time and peak RSS (`wait4`) are taken by `dirbench run` inside the container of
the build. A build that fails or times out in its warm-up is not run again on that query;
a failed timed run is reported and the other runs still count.

### Reading the output

The job summary has one row per query:

- `base median s`, `base min s`, `head median s`, `head min s`: wall time over the timed
  runs that succeeded.
- `median head/base`, `min head/base`: below `1.00x` head is faster. The minimum is the
  steadier number on a shared runner; a difference of a few percent is noise.
- `base RSS MiB`, `head RSS MiB`, `RSS head/base`: the largest peak RSS of the timed runs.
- `output`: `same` when the last successful run of both builds wrote the same bytes,
  `DIFFERENT` when it did not (a speed-up that changes the result is not one), `-` when
  one build has no successful run.
- `status`: `ok`, or `FAILED` with the build, the status (`timeout`, `exit-N`, `not-run`)
  and the run (0 is the warm-up). A failing query is never left out. Each one is also a
  warning annotation of the run. The job itself ends green when the comparison ran; read
  the status column.

The artifact `bench-compare` has `table.md` (the same table), `raw.tsv` (every run: query,
build, run, wall ms, peak RSS KiB, status, output checksum), `errors/` (stderr of the last
run of each query and build) and `runner.txt` (commits, inputs, machine).

### Recording results

A run is not a result until it is in the repository. Copy the table and the run id into
the suite's `RESULTS.md` in a pull request, with the commits compared. The workflow does
not write to the repository.

### Testing the scripts

`test/scripts/bench_compare.cmake` (ctest `scripts/bench_compare`) runs
`compare_table.xq` on a sample of raw results, and on Linux `compare.sh` end to end
with a passing, a failing, a timing out and an argument-taking query.

## XMark and JSONiq (`suite` empty)

`bench.yml` without `suite` measures `xmark/` and `json/` with `run.py`: weekly on master,
or dispatched with `ref_a` (default: the ref) and optionally `ref_b`, which reports
`ref_b / ref_a`; `runs` is the number of timed runs, `timeout` the limit per query.
