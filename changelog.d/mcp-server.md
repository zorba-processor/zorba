### Added

- `zorba-mcp --root <dir> [--queries <dir>] [--timeout <seconds>] [--max-output <bytes>]`: an MCP server (JSON-RPC 2.0 over stdio) that lets an AI agent query a directory of XML, JSON and CSV documents. It implements MCP 2026-07-28 (per-request `_meta`, `server/discover`) and, after an `initialize` request, the handshake revisions 2025-11-25, 2025-06-18 and 2025-03-26. Tools: `list_documents`, `run_query` (a JSONiq/XQuery query with the directory module's `$dir:root` bound; the result is JSON, one item per line) and one tool per `<name>.jq` file in `--queries` (its leading comment is the description, its external variables are string parameters). A failed query returns the Zorba error code, message and the line and column of the failing expression. Output longer than `--max-output` (default 128 KiB) is cut with a marker; each call is limited to `--timeout` seconds (default 30).

### Security

- `zorba-mcp` runs every query under the same policy as `zorba --directory`: the secure policy with only `file-read` (restricted to the root) and `native-modules` allowed.
