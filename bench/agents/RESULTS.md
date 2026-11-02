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
