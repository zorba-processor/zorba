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

## 2026-11-03, commits 7c8d649f9 and 27e952e2f (`list_documents` shorter)

Backend `claude-code`, model `claude-sonnet-5-5`, `--language jsoniq`, 20 tasks,
3 runs per task and configuration: 60 sessions per cell. 7c8d649f9 is master,
run 41646733315. 27e952e2f adds the shorter `list_documents` listing to that
master, run 41646738034. Per-task tables and the per-run records are in
[`baselines/2026-11-03-7c8d649f9-jsoniq/`](baselines/2026-11-03-7c8d649f9-jsoniq/)
and
[`baselines/2026-11-03-27e952e2f-jsoniq/`](baselines/2026-11-03-27e952e2f-jsoniq/).
Tokens are means per session; reported USD is the sum over the 60 sessions of
the cost that Claude Code reports.

| Config | Measure | 7c8d649f9 | 27e952e2f |
|---|---|---:|---:|
| A | accuracy | 1.00 | 1.00 |
| A | tool calls | 3.7 | 3.9 |
| A | tokens, mean | 23983 | 25108 |
| A | tokens, median | 19762 | 17988 |
| A | wall s | 19.4 | 18.5 |
| A | reported USD | 2.1899 | 2.1799 |
| A | failed `run_query` | 61 of 161 | 67 of 171 |
| B | accuracy | 1.00 | 0.98 |
| B | tokens, mean | 15436 | 13499 |
| B | tokens, median | 11346 | 10812 |
| B | wall s | 13.8 | 12.7 |
| B | reported USD | 0.9715 | 0.9885 |

The listing went from 4671 to 3986 characters (57 calls per run) and with
`path=weather` from 2629 to 2574 (3 calls per run).

**These two runs do not show a saving from the shorter listing.** Comparing the
mean per task of 27e952e2f with that of 7c8d649f9:

| Config | Tasks lower | Tasks higher | Median difference | Mean difference |
|---|---:|---:|---:|---:|
| A | 15 | 5 | -2344 | +1125 |
| B | 11 | 7 | -7 | -1936 |

B has no listing and no code change between the two runs, and its mean moved
12.5%. That is larger than the 4.2% stated for the earlier reruns, and larger
than the difference in A. The one wrong answer is `t13_cyclone_categories`,
run 2, in B.

### Query errors, both runs together

128 of 332 `run_query` calls failed, in 74 of the 120 A sessions (62%), by
cause:

| Cause | Failed |
|---|---:|
| `.` as the context item (JSONiq uses `$$`) | 39 |
| String lookup on an array (FORG0001) | 25 |
| `math:` prefix not declared | 11 |
| Returning a node (JNSE0014) | 10 |
| `$a-$b` without spaces | 10 |
| Other syntax | 8 |
| Singleton expected | 8 |
| `number` or `xs:null` as a type | 4 |
| Names that do not exist | 4 |
| `\s` in a string, backslash not escaped | 4 |
| `[n]` on an array | 3 |
| Single quotes | 2 |

16 repeat a cause already seen in the session and 37 come right after another
failure.

| Failed queries in the session | Sessions | Median tokens |
|---:|---:|---:|
| 0 | 46 | 11447 |
| 1 | 45 | 18299 |
| 2 | 15 | 32834 |
| 3 | 8 | 38432 |
| 4 or more | 6 | 71340 |

Harder tasks fail more, so this table overstates the cost of an error. Within
one task, sessions with an error use a median of 1.46 times the tokens of
sessions without one (14 tasks have both kinds). Over the 15 tasks with a
session without errors, those sessions use a median of 1.19 times the tokens
of B; over all sessions of both runs A uses 1.70 times the tokens of B.

Limits: 3 runs per task, one model, 1.8 MB of data. The cache counters of these
runs cannot be used: `zorba-mcp.log` keeps one process, with no queries.

## 2026-11-03, commits 3d8fbf0cf and 8648d2de5 (JSONiq primer and query hints)

Backend `claude-code`, model `claude-sonnet-5-5`, `--language jsoniq`, 20 tasks,
3 runs per task and configuration: 60 sessions per cell. 3d8fbf0cf is master,
run 41704489783. 8648d2de5 adds the 687-byte JSONiq primer in the `run_query`
description and the query hints to the parent of 3d8fbf0cf, which only adds
benchmark records, run 41704494854. Per-task tables and the per-run records are in
[`baselines/2026-11-03-3d8fbf0cf-jsoniq/`](baselines/2026-11-03-3d8fbf0cf-jsoniq/)
and
[`baselines/2026-11-03-8648d2de5-jsoniq/`](baselines/2026-11-03-8648d2de5-jsoniq/).
Tokens are means per session unless stated; reported USD is the sum over the 60
sessions of the cost that Claude Code reports. Every figure is recomputed from
`runs.jsonl` of the two runs.

| Config | Measure | 3d8fbf0cf | 8648d2de5 |
|---|---|---:|---:|
| A | accuracy | 1.00 | 1.00 |
| A | tool calls | 3.6 | 3.0 |
| A | tokens, mean | 22104 | 19315 |
| A | tokens, median | 18892 | 17456 |
| A | wall s | 19.1 | 14.4 |
| A | reported USD | 2.0463 | 1.8268 |
| A | failed `run_query` | 52 of 155 | 27 of 121 |
| A | sessions with a failed query | 36 of 60 | 20 of 60 |
| B | accuracy | 1.00 | 1.00 |
| B | tokens, mean | 13982 | 13916 |
| B | tokens, median | 11987 | 11402 |
| B | wall s | 12.0 | 13.5 |
| B | reported USD | 0.8361 | 1.0428 |

Failed `run_query` calls without the primer in the earlier runs: 61 of 161
(7c8d649f9) and 67 of 171 (27e952e2f).

Comparing the failed queries per task of 8648d2de5 with those of 3d8fbf0cf, 12
tasks have fewer, 2 have more (`t17_gdacs_eq_match` 2 to 5,
`t18_hot_cities_near_alerts` 2 to 5) and 6 are the same.

**These two runs do not show a token saving.** Comparing the mean tokens per
task of A:

| Config | Tasks lower | Tasks higher | Median difference | Mean difference |
|---:|---:|---:|---:|---:|
| A | 10 | 10 | -54 | -2789 |

The mean falls mostly on a few tasks (`t14_fires_near_quakes` alone is -27612 of
the -55771 that the 20 differences add up to), and half of the tasks went up. No
token saving is claimed.

### Failed queries by code

| code | 3d8fbf0cf | 8648d2de5 |
|---|---:|---:|
| `err:XPST0003` | 24 | 15 |
| `err:FORG0001` | 13 | 0 |
| `jerr:JNSE0014` | 7 | 2 |
| `err:XPTY0004` | 2 | 5 |
| `err:XPST0051` | 2 | 0 |
| `jerr:JNTY0024` | 1 | 3 |
| `err:FORG0006` | 1 | 2 |
| `jerr:JNTY0004` | 1 | 0 |
| `err:XPST0081` | 1 | 0 |
| total | 52 | 27 |

The 15 `err:XPST0003` of 8648d2de5, by cause, from reading each query:

| Cause | Failed | Hint given |
|---|---:|---|
| `.` as the context item | 11 | all 11 |
| `\s` in a string, backslash not escaped | 3 | all 3 |
| `$x[1:1]`, a slice JSONiq does not have | 1 | none |

The primer states the first two, and agents still wrote them; 14 of the 15 were
followed by a query that succeeded, and the other (the slice) by a `.` failure
and then a success.

Limits: 3 runs per task, one model, 1.8 MB of data. The primer and the hints went
in together, so these runs do not separate them. The tasks are the ones the list
of common errors came from, so the fall in failures may not carry over to other
tasks. Everything here is `--language jsoniq`; no run measures `--language
xquery` with this work.

The measured commit predates the review of this work. A later commit keeps the
primer text and the instructions of a JSONiq session byte for byte, and an XQuery
session no longer has either, which these runs did not measure. These later
changes can affect a JSONiq session, and were not measured:

- The hint on `\` also lists `\'`, and also fires on a literal over several
  lines. The 3 `\s` failures above got the old text.
- The hint on `.` fires only where an expression can start. All 11 `.` failures
  above are in such a place, so they would get it.
- A misspelled Zorba math namespace (`http://www.zorba.io/modules/math`) points
  to `http://zorba.io/modules/math` instead of the W3C namespace, and any other
  namespace that ends in `/math` gets no math pointer. No query of run 41704494854
  imports such a namespace; 7c8d649f9 and 27e952e2f have 3 and 2 that do.
- A function of the W3C namespace called with the prefix `math` bound to another
  namespace gets different advice, and a wrong arity of a declared function comes
  before the math pointer.
- A selector computed at run time gets a placeholder in the hint of a key on an
  array. Run 41704494854 has no `err:FORG0001`.
- `$a--` gets `$a - -...`.
