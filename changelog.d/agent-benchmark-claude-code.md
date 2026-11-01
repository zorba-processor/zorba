### Added

- The agent benchmark (`bench/agents/`) can drive the model with Claude Code
  headless (`harness.py --backend claude-code`, authenticated with the
  runner's own `CLAUDE_CODE_OAUTH_TOKEN`) besides the Anthropic API
  (`--backend api`, with the runner's own `ANTHROPIC_API_KEY`). In the
  `claude-code` backend the zorba-mcp configuration has no built-in tools and
  the bash configuration runs Claude Code inside the sandbox container, whose
  only network route is a proxy that allows the Anthropic API; runs also
  record the cost Claude Code reports. The `Agent benchmark` workflow chooses
  the backend at dispatch, and its pull-request dry run covers both backends.

### Changed

- The agent benchmark's JSONiq ground-truth queries read GeoJSON, HTML and
  several CSV files through the directory module (`dir:open`,
  `dir:documents`) now that it handles them.
