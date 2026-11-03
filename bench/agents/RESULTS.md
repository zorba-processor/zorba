# Agent benchmark results

## 2026-11-02, commit 31994632c (baseline)

Model `claude-sonnet-5-5`, 20 tasks, 3 runs per task and configuration: 120
sessions per backend. A = `zorba-mcp` tools only; B = bash sandbox only.
Tokens include cache reads and writes. Per-task tables and the per-run records
are in [`baselines/2026-11-02-31994632c/`](baselines/2026-11-02-31994632c/).

| Backend | Run | Config | Accuracy | Tool calls | Tokens | Wall s | Repeatability |
|---|---|---|---:|---:|---:|---:|---:|
| claude-code | 41443994027 | A | 1.00 | 4.2 | 24829 | 20.7 | 0.97 |
| claude-code | 41443994027 | B | 1.00 | 2.5 | 13834 | 12.9 | 1.00 |
| api | 41444947221 | A | 1.00 | 7.2 | 218642 | 18.3 | 0.95 |
| api | 41444947221 | B | 1.00 | 3.1 | 11659 | 7.9 | 0.98 |

Accuracy does not separate the configurations: both answered every task
correctly. On every efficiency measure B is ahead. No task used fewer tokens in
A on the `api` backend.

Causes measured in configuration A:

- **Query errors.** 79 of 194 `run_query` calls failed on `claude-code` and 157
  of 372 on `api`. The most frequent: `$x[]` (`unexpected "]"`), `keys` and
  similar functions without a prefix, `$a-$b` without spaces, undeclared
  prefixes such as `math:`, and on `api` serializing a node as JSON (JNSE0014).
- **Large results while exploring.** On `api`, 35 calls returned 100 KB or more,
  in 30 of the 60 sessions. Those sessions averaged 393482 tokens and the other
  30 averaged 43803, because each result stays in the context of every later
  turn. The two backends limit a result differently (128 KiB on `api`, 50000 on
  `claude-code`), which explains much of the difference between them.

Limits: the snapshot is 1.8 MB, a size where short Python scripts answer every
task, and the tasks do not measure behavior without a shell or at larger scale.

## 2026-11-02, commit c22ed8454 (rerun, `--language xquery` and `jsoniq`)

Backend `claude-code`, model `claude-sonnet-5-5`, 20 tasks, 3 runs per task and
configuration, one rerun per query language of configuration A: xquery is run
41551872837, jsoniq is run 41551893301. Configuration B does not use the
language option; it ran in both reruns. Per-task tables and the per-run records
are in
[`baselines/2026-11-02-c22ed8454-xquery/`](baselines/2026-11-02-c22ed8454-xquery/)
and
[`baselines/2026-11-02-c22ed8454-jsoniq/`](baselines/2026-11-02-c22ed8454-jsoniq/).
Every figure below is recomputed from `runs.jsonl` of the three runs (baseline
31994632c is run 41443994027, `claude-code` backend). Means are per session
(60 sessions per cell); repeatability is the mean over tasks of the share of
runs that gave the most common answer; reported USD is the sum over the 60
sessions of the `total_cost_usd` that Claude Code reports.

| Config | Measure | baseline 31994632c | xquery | jsoniq |
|---|---|---:|---:|---:|
| A | accuracy | 1.00 | 0.98 | 1.00 |
| A | tool calls | 4.2 | 3.5 | 3.5 |
| A | turns | 6.2 | 5.5 | 5.5 |
| A | tokens | 24829 | 22696 | 22860 |
| A | wall s | 20.7 | 11.1 | 10.8 |
| A | repeatability | 0.97 | 0.93 | 0.95 |
| A | reported USD | 2.2077 | 2.1363 | 2.1468 |
| B | accuracy | 1.00 | 1.00 | 1.00 |
| B | tool calls | 2.5 | 2.5 | 2.6 |
| B | turns | 4.5 | 4.5 | 4.6 |
| B | tokens | 13834 | 14065 | 14658 |
| B | wall s | 12.9 | 6.8 | 6.9 |
| B | repeatability | 1.00 | 0.98 | 0.97 |
| B | reported USD | 1.0905 | 1.0933 | 0.7957 |
| A/B | token ratio | 1.79 | 1.61 | 1.56 |

What the data shows:

- The A to B token ratio is 1.61 (xquery) and 1.56 (jsoniq); in the baseline it
  was 1.79. A is above B by more than half in both reruns. The comparative
  criterion is not met on tokens, tool calls, turns or reported USD (B is lower
  on all four in both reruns).
- Setting `--language jsoniq` changed the tokens of A by +0.7% (22696 to
  22860). B, which does not use the option, differs by +4.2% between the same
  two runs (14065 to 14658), so run-to-run variation on B is larger than the
  effect on A.
- Accuracy differences are one task in one run: A in the xquery run answered
  `t13_cyclone_categories` wrong in run 2 (59 of 60 sessions correct); every
  other session in both reruns was correct.
- Wall times of the reruns are not comparable with the baseline: both A and B
  fell by about half (A 20.7 to 11.1 and 10.8 s, B 12.9 to 6.8 and 6.9 s). B
  does not use the language option, so the change in B is not an effect of the
  option; what changed it is not recorded in these files.

### Query errors in configuration A

A `run_query` call counts as an error when its output is an `{"error": ...}`
object (this equals the `is_error` flag of the tool log in all three runs).
The baseline figure of 79 of 194 counted `run_query` calls only; the table
gives both denominators.

| | baseline 31994632c | xquery | jsoniq |
|---|---:|---:|---:|
| `run_query` calls | 194 | 149 | 150 |
| `list_documents` calls | 60 | 60 | 60 |
| `run_query` calls with an error | 79 | 49 | 52 |
| rate over `run_query` calls | 0.41 | 0.33 | 0.35 |
| rate over all tool calls | 79/254 = 0.31 | 49/209 = 0.23 | 52/210 = 0.25 |
| `run_query` calls starting with `jsoniq version` | 0 of 194 | 55 of 149 | 0 of 150 |

In the xquery run, 15 of the 55 queries with the declaration failed and 34 of
the 94 without it. The error codes by count:

| code | baseline | xquery | jsoniq |
|---|---:|---:|---:|
| `err:XPST0003` | 36 | 29 | 28 |
| `err:FORG0001` | 9 | 8 | 11 |
| `err:XPST0017` | 14 | 0 | 2 |
| `err:XPST0008` | 5 | 0 | 2 |
| `jerr:JNSE0014` | 5 | 4 | 4 |
| `err:XPST0051` | 1 | 5 | 2 |
| `err:XPST0081` | 4 | 0 | 0 |
| `err:XPTY0019` | 4 | 0 | 0 |
| `err:XPTY0004` | 0 | 3 | 0 |
| `jerr:JNTY0004` | 0 | 0 | 2 |
| `jerr:JNTY0024` | 1 | 0 | 1 |
| total | 79 | 49 | 52 |

With `--language jsoniq`, no query carries a `jsoniq version` declaration (0 of
150 against 55 of 149) and the error count did not fall (52 against 49).

### Tokens per session, A versus B

Mean per session, from the `usage` field. Input tokens (uncached) are 7 to 10
per session and not shown.

| Run | Config | cache creation | cache read | output | total |
|---|---|---:|---:|---:|---:|
| baseline | A | 5685 | 17910 | 1224 | 24829 |
| baseline | B | 2853 | 10403 | 571 | 13834 |
| baseline | A minus B | +2832 | +7507 | +653 | +10995 |
| xquery | A | 6061 | 15648 | 978 | 22696 |
| xquery | B | 2877 | 10617 | 564 | 14065 |
| xquery | A minus B | +3184 | +5031 | +414 | +8631 |
| jsoniq | A | 6025 | 15818 | 1008 | 22860 |
| jsoniq | B | 1562 | 12514 | 575 | 14658 |
| jsoniq | A minus B | +4463 | +3304 | +433 | +8202 |

B in the jsoniq run has about half the cache creation of B in the xquery run
(1562 against 2877) and more cache reads (12514 against 10617), with the same
configuration and the same total within 4.2%. The files do not say why.

### `list_documents` output

A calls `list_documents` once per session: 60 calls per run. The tool log keeps
only the first 4000 characters of each result (`harness.py`) and records the
full length in `output_bytes`.

| | baseline 31994632c | xquery | jsoniq |
|---|---:|---:|---:|
| calls | 60 | 60 | 60 |
| full output, characters | 2357 (all 60) | 4671 (57), 2629 (3) | 4671 (57), 2629 (3) |
| mean full output | 2357 | 4569 | 4569 |
| logged output (cut at 4000) | 2357 | mean 3931 | mean 3931 |

The 3 calls with 2629 characters report `"total":12,"shown":12` in their first
line with `"files_found":21` and `"state":"complete"`; the other 57 report
`"total":21,"shown":21` in 4671 characters. The 3931 figure is the mean of the logged text, cut at
4000 by the harness; the text the model received is 4671 characters in 57 calls.
The listing is about twice as long as in the baseline. Whether that length
accounts for part of the difference in cache creation and cache read between A
and B is a hypothesis; these runs have no variant with a shorter listing.

Limits: one rerun per language, 3 runs per task, one model; B differs between
two runs by up to 4.2% in tokens with no configuration change.
