# Agent benchmark: zorba-mcp versus bash

This benchmark measures whether an AI agent answers questions over a
directory of XML, JSON, CSV and HTML files better with Zorba's MCP server
than with a shell. The comparison is the success criterion: the same model,
the same tasks and the same directory, in two configurations:

- **A**: the `zorba-mcp` tools only (`list_documents`, `run_query`), over
  MCP/stdio.
- **B**: a `bash` tool only, in a container with `python3` (standard library),
  `jq`, `xmllint`, `grep`, `awk`, `sed` and `find`.

For every task and run the harness records correctness against the ground
truth, the number of tool calls and model turns, input and output tokens
(including cache reads and writes), a cost estimate, wall time, and the
answer itself, so that repeatability (the same answer across N runs) can be
computed.

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

- Same model, system prompt, task prompt, turn budget (30 model requests),
  `max_tokens` per response (16000), default sampling and thinking settings,
  and the same `submit_answer` tool in both configurations. Only the data
  tools and their descriptions differ.
- Same limits on tool calls: 30 seconds and 128 KiB of output per call
  (`zorba-mcp --timeout 30 --max-output 131072`; `timeout 30` and the same
  cut in the sandbox).
- Both run without network and with the snapshot read-only: zorba-mcp
  enforces its secure policy (only files under the root can be read); the
  bash sandbox is a container with `--network none`, a read-only root file
  system, a 64 MB `/tmp`, 1 CPU, 1 GB of memory and 256 processes.
- Configuration A gets no predefined query that answers a task: the
  `queries/` directory is never passed as `--queries`, and the harness
  refuses a `--queries-dir` that contains a task's query file.
- Each run starts a fresh zorba-mcp process or sandbox container.
- No refusal fallback to another model: a refusal is recorded as such, since
  a fallback would change the model in the middle of a run.
- An agent that ends without calling `submit_answer` is graded on the last
  JSON code block of its text, if any, in both configurations.

## Running

The real benchmark runs in GitHub Actions: workflow **Agent benchmark**,
"Run workflow", with the inputs model (default `claude-sonnet-5-5`), runs
per task and configuration (default 3), configuration (`both`, `A`, `B`) and
an optional task filter. It needs the repository secret `ANTHROPIC_API_KEY`
and fails at its first step without it. Results are uploaded as the
`agent-benchmark-results` artifact (`runs.jsonl`, `summary.json`,
`summary.md`) and the summary table is shown on the run page.

On pull requests that touch `bench/agents/`, the same workflow builds Zorba,
runs `check_truth.py` and runs the harness with `--dry-run` in both
configurations.

Locally, with a built `zorba-mcp` on the `PATH` and Docker:

```sh
docker build -t zorba-agent-sandbox:latest bench/agents/sandbox
python3 bench/agents/harness.py --dry-run            # no API key needed
pip install -r bench/agents/requirements.txt
ANTHROPIC_API_KEY=... python3 bench/agents/harness.py --runs 3 \
    --config both --tasks t01,t14 --out /tmp/agent-results
```

Options: `--model`, `--runs`, `--config A|B|both`, `--tasks`, `--effort`,
`--max-turns`, `--max-tokens`, `--tool-timeout`, `--max-output`,
`--mcp-command` (how to start zorba-mcp; `{snapshot}` is replaced),
`--sandbox-image`, `--queries-dir` (predefined query tools for A), `--out`.

The dry run replaces the model with a script: in A it runs the task's
JSONiq query through zorba-mcp's `run_query`, in B it runs `reference.py`
through the bash tool inside the sandbox, then submits the result. It tests
the loop, both tool paths and the grading, and every dry run must be graded
correct.

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
| `harness.py` | runs the agents and writes the results |
| `sandbox/Dockerfile` | the bash sandbox of configuration B |
| `requirements.txt` | the Anthropic SDK, for real runs only |

Python is used here only because this directory is a test harness: it is not
built by CMake and not shipped.
