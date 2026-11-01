# Agent benchmark: zorba-mcp versus bash

This benchmark measures whether an AI agent answers questions over a
directory of XML, JSON, CSV and HTML files better with Zorba's MCP server
than with a shell. The comparison is the success criterion: the same model,
the same tasks and the same directory, in two configurations:

- **A**: the `zorba-mcp` tools only (`list_documents`, `run_query`), over
  MCP/stdio.
- **B**: a `bash` tool only, in a container with `python3` (standard library),
  `jq`, `xmllint`, `grep`, `awk`, `sed` and `find`.

Two backends drive the model. Whoever runs the benchmark supplies their own
credentials; the repository provides none:

- **`api`**: the Anthropic Messages API through the Python SDK, with a manual
  tool loop. Needs your own `ANTHROPIC_API_KEY`.
- **`claude-code`**: Claude Code headless (`claude -p`), billed to your Claude
  subscription. Needs `CLAUDE_CODE_OAUTH_TOKEN`, which `claude setup-token`
  prints.

For every task and run the harness records correctness against the ground
truth, the number of tool calls and model turns, input and output tokens
(including cache reads and writes), a cost estimate from list prices, wall
time, and the answer itself, so that repeatability (the same answer across N
runs) can be computed. The `claude-code` backend also records the
`total_cost_usd` that Claude Code reports (an estimate, not billing data),
the API time and the permission denials.

## Use case

A situational-awareness analyst working on a frozen snapshot of public
geospatial feeds, in the spirit of globe dashboards that combine public
feeds (for example the open-source God's Eye View). The snapshot is
`data/snapshot/` (about 1.8 MB, committed; the benchmark never fetches data):

| Path | Format | Source | Terms |
|---|---|---|---|
| `earthquakes/usgs_m2.5_week.geojson` | GeoJSON | USGS M2.5+ past week | public domain |
| `earthquakes/usgs_m2.5_week.csv` | CSV | the same feed, fetched 120 min later | public domain |
| `earthquakes/usgs_m4.5_week_quakeml.xml` | QuakeML (XML) | USGS M4.5+ past week, fetched 60 min later | public domain |
| `satellites/celestrak_stations_omm.xml` | CCSDS OMM XML | CelesTrak, group "stations" | free, attribution |
| `satellites/celestrak_stations.csv` | CSV | the same group | free, attribution |
| `alerts/gdacs_rss.xml` | RSS (XML) | GDACS disaster alerts | reuse with acknowledgment |
| `fires/firms_modis_*_24h.csv` | CSV | NASA FIRMS MODIS 24 h, cut to two regions | NASA open data |
| `weather/*.json`, `weather/cities.csv` | JSON, CSV | Open-Meteo current and daily weather for 11 cities; city index written for the benchmark | CC BY 4.0; CC0 |
| `reference/nhc_saffir_simpson_scale.html` | HTML | NOAA NHC Saffir-Simpson scale page | public domain |

`data/SOURCES.md` gives the URL, retrieval time, size, SHA-256 and terms of
every file. Two properties are deliberate and documented there:

- The three earthquake files were fetched 60 minutes apart, so they disagree
  the way copies of a live feed do (new events, events leaving the one-week
  window, revised magnitudes).
- `weather/anchorage.json` is truncated (not valid JSON). Task t16 can only
  be answered correctly by noticing it.

`fetch_snapshot.py` rebuilds the snapshot (it takes two hours because of the
staggered fetches). A new snapshot changes the answers: run
`reference.py --write` afterwards and check that the ground truth is still
unambiguous (`reference.py` refuses data where a value sits on a task
threshold or two candidates tie).

## Tasks

`tasks/tasks.json` holds 20 tasks: the question, the JSON schema of the
answer and a grading rule (`grading.py`: exact match, numeric tolerance, set
equality, keyed sets of objects, ordered lists). They cover lookups,
aggregations, cross-format consistency (GeoJSON versus CSV versus QuakeML,
units), HTML table extraction, cross-source joins with haversine distances,
orbital arithmetic and the malformed file.

The ground truth is computed twice, independently:

1. `reference.py`: Python, standard library only. `reference.py --write`
   stores its answers in `tasks/ground_truth.json`.
2. `queries/<task>.jq`: JSONiq run by `zorba --directory data/snapshot`.

`check_truth.py --zorba <path>` grades each query's result against the
reference with the task's own rule; CI fails if any task disagrees or
`ground_truth.json` is stale.

## Fairness rules

Common to both backends:

- Same model, system prompt, task prompt, turn budget (30), maximum output
  tokens per response (16000) and default sampling and thinking settings in
  A and B, and the same way of giving the final answer: the `submit_answer`
  tool (`api`) or structured output validated against the answer schema
  (`claude-code`, `--json-schema`). Only the data tools differ.
- The snapshot is read-only in both: zorba-mcp enforces its secure policy
  (only files under the root can be read); the bash
  sandbox is a container with the snapshot mounted read-only, a read-only
  root file system, a 64 MB `/tmp`, 1 CPU and memory and process limits.
- Configuration A gets no predefined query that answers a task: the
  `queries/` directory is never passed as `--queries`, and the harness
  refuses a `--queries-dir` that contains a task's query file.
- Each run starts a fresh zorba-mcp process or sandbox container.
- No refusal fallback to another model: a refusal is recorded as such.
- An agent that ends without giving its answer through the answer mechanism
  is graded on the last JSON code block of its text, if any.

### `api` backend

- Per tool call: 30 seconds and 128 KiB of output (`zorba-mcp --timeout 30
  --max-output 131072`; `timeout 30` and the same cut in the sandbox).
- The bash sandbox runs with `--network none`, 1 GB of memory and 256
  processes; the harness runs each command in it with `docker exec`.
- The turn budget is 30 model requests.

### `claude-code` backend

Every rule is enforced by Claude Code options, environment variables or the
container, not by prompt text:

| Rule | A (zorba-mcp) | B (bash) |
|---|---|---|
| Where claude runs | on the host | inside the sandbox container |
| Built-in tools | none: `--tools ""` | `--tools Bash` |
| MCP servers | zorba-mcp only: `--mcp-config`, `--strict-mcp-config` | none: `--strict-mcp-config` and no `--mcp-config` |
| Permissions | `--permission-mode dontAsk --allowedTools mcp__zorba` | `--permission-mode dontAsk --allowedTools Bash` |
| Network of the tools | zorba-mcp's secure policy; in the workflow it also runs in a container with `--network none` | internal Docker network; the only route out is `egress_proxy.py`, which allows CONNECT to `api.anthropic.com:443` and refuses everything else |
| Time per call | `zorba-mcp --timeout 30` (`MCP_TOOL_TIMEOUT` 60 s as a backstop) | `BASH_DEFAULT_TIMEOUT_MS` = `BASH_MAX_TIMEOUT_MS` = 30000, `CLAUDE_CODE_DISABLE_BACKGROUND_TASKS=1` |
| Output per call | `zorba-mcp --max-output 50000` (bytes; `MAX_MCP_OUTPUT_TOKENS` 50000 so Claude Code does not cut earlier) | `BASH_MAX_OUTPUT_LENGTH` and `bashOutputMaxChars` 50000 (characters) |

In both: `--model`, `--system-prompt` (replaces Claude Code's own system
prompt), `--max-turns 30`, `--json-schema`, `CLAUDE_CODE_MAX_OUTPUT_TOKENS`,
`--effort` when given, a fresh configuration directory (A:
`CLAUDE_CONFIG_DIR` in a temporary directory; B: an empty tmpfs home),
`CLAUDE_CODE_DISABLE_CLAUDE_MDS=1`, `CLAUDE_CODE_DISABLE_AUTO_MEMORY=1`,
`--no-session-persistence`, and `ANTHROPIC_API_KEY` / `ANTHROPIC_AUTH_TOKEN`
removed from claude's environment (they would take precedence over the OAuth
token). The harness reads the tool list of Claude Code's `init` event in every
run and stops if it is not exactly the expected set (A: `StructuredOutput` and
the `mcp__zorba__*` tools; B: `StructuredOutput` and `Bash`).

Differences from the `api` backend and limits of the enforcement:

- Output per tool call is 50000 bytes (A) or characters (B), not 128 KiB:
  Claude Code saves a successful MCP result of more than 50,000 characters to
  a file and shows the model a preview, and A has no tool to read that file.
  The harness refuses `--max-output` above 50000 for this backend.
- B's output limit counts characters, A's counts bytes. Bash output longer
  than the limit is saved by Claude Code to a file in the container with a
  preview of about 2,000 characters, which B can read with further commands;
  A gets zorba-mcp's truncated result.
- In B, the commands can reach `api.anthropic.com` through the proxy (Claude
  Code needs it); no other host. In A, the claude process itself has the
  host's network, but A has no tool that uses it.
- The B container has 2 GB of memory and 512 processes (Claude Code runs in
  it too), against 1 GB and 256 for the `api` sandbox. The B image contains
  the Claude Code binary besides the data tools; Node.js is not installed.
- `--max-turns` counts Claude Code turns; the `StructuredOutput` call that
  gives the answer is part of them. The `api` backend counts model requests.
- Claude Code does not report per-call durations, so the tool log of this
  backend has `"seconds": null`; the per-run wall time and
  `duration_api_ms` are recorded.
- `total_cost_usd` is Claude Code's own estimate from its price table; under
  a subscription it is not what is billed.

## Running

The real benchmark runs in GitHub Actions: workflow **Agent benchmark**, "Run
workflow", with the inputs backend (`claude-code` or `api`), model (default
`claude-sonnet-5-5`), runs per task and configuration (default 3),
configuration (`both`, `A`, `B`) and an optional task filter. Whoever runs it
stores their own credentials as a repository secret first:

- `claude-code`: run `claude setup-token` (with a Claude subscription) and
  store the token as `CLAUDE_CODE_OAUTH_TOKEN`.
- `api`: store your Anthropic API key as `ANTHROPIC_API_KEY`.

The run fails at its first step if the secret of the chosen backend is
missing, and passes each secret only to the backend that uses it. Results
are uploaded as the `agent-benchmark-results` artifact (`runs.jsonl`,
`summary.json`, `summary.md`, and for `claude-code` also
`claude-stderr.log` and `egress-proxy.log`), and the summary table is shown
on the run page. The workflow pins Claude Code with `CLAUDE_CODE_VERSION`.

On pull requests that touch `bench/agents/` or the workflow, the same
workflow builds Zorba, runs `check_truth.py`, checks that the pinned Claude
Code accepts the harness's command line (`--check-cli`: the real `claude`,
without credentials or network) and runs the harness with `--dry-run` for
both backends in both configurations.

Locally, with a built `zorba-mcp` on the `PATH` and Docker:

```sh
docker build --target sandbox -t zorba-agent-sandbox:latest bench/agents/sandbox
docker build --target sandbox-claude --build-arg CLAUDE_CODE_VERSION=2.1.302 \
    -t zorba-agent-sandbox-claude:latest bench/agents/sandbox
python3 bench/agents/harness.py --backend api --dry-run          # no credentials
python3 bench/agents/harness.py --backend claude-code --dry-run  # no credentials

# claude-code: a Claude subscription and Claude Code installed on the host
# (npm install -g --allow-scripts=@anthropic-ai/claude-code
# @anthropic-ai/claude-code@2.1.302). `claude setup-token`
# prints a token; the harness passes it to claude in both configurations
# (in B claude runs in the container, where your login is not available).
CLAUDE_CODE_OAUTH_TOKEN=... python3 bench/agents/harness.py \
    --backend claude-code --runs 3 --config both --tasks t01,t14 \
    --out /tmp/agent-results

# api: your own API key
pip install -r bench/agents/requirements.txt
ANTHROPIC_API_KEY=... python3 bench/agents/harness.py --backend api \
    --runs 3 --config both --tasks t01,t14 --out /tmp/agent-results
```

Options: `--backend api|claude-code`, `--model`, `--runs`, `--config
A|B|both`, `--tasks`, `--effort`, `--max-turns`, `--max-tokens`,
`--tool-timeout`, `--max-output`, `--mcp-command` (how to start zorba-mcp;
`{snapshot}` is replaced), `--sandbox-image`, `--queries-dir` (predefined
query tools for A), `--out`; for `claude-code` also `--claude-command` (claude
on the host, A), `--claude-image` (B), `--egress-allow`, `--run-timeout` and
`--check-cli`.

The dry run replaces the model with a script: in A it runs the task's
JSONiq query through zorba-mcp's `run_query`, in B it runs `reference.py`
through the bash tool inside the sandbox, then gives the result as the
answer. For `api` the script replaces the SDK responses; for `claude-code`
it is `stub_claude.py`, which gets the same command line, environment and
container as the real `claude`, refuses a command line without the fairness
options, writes Claude Code's stream-json events, and in B first checks the
sandbox (read-only `/data`, no direct route out, the proxy refusing a feed
host and allowing the Anthropic API). Every dry run must be graded correct.

## Files

| File | Purpose |
|---|---|
| `fetch_snapshot.py` | one-off fetch of `data/snapshot/` and `data/SOURCES.md` |
| `tasks/tasks.json` | questions, answer schemas, grading rules |
| `tasks/ground_truth.json` | answers written by `reference.py --write` |
| `reference.py` | Python ground truth |
| `queries/*.jq` | JSONiq ground truth |
| `check_truth.py` | checks that the two ground truths agree |
| `grading.py`, `test_grading.py` | grading rules and their tests |
| `harness.py` | runs the agents (both backends) and writes the results |
| `stub_claude.py` | stand-in `claude` of the `claude-code` dry run |
| `egress_proxy.py` | allowlisting proxy of configuration B (`claude-code`) |
| `test_harness.py` | tests of the `claude-code` command line, the stream reader and the proxy |
| `sandbox/Dockerfile` | the sandbox of configuration B (`sandbox`; `sandbox-claude` adds Claude Code) |
| `requirements.txt` | the Anthropic SDK, for real runs of the `api` backend only |

Python is used here only because this directory is a test harness: it is not
built by CMake and not shipped.
