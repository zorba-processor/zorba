### Added

- The `ghcr.io/zorba-processor/zorba` image runs `zorba-mcp`: `docker run -i --rm -v "$PWD:/data:ro" ghcr.io/zorba-processor/zorba zorba-mcp --root /data` serves a folder of XML, JSON and CSV files to an MCP client. Any other arguments still go to `zorba`. The image workflow smoke-tests the server (`initialize`, `server/discover`, `tools/list`, `tools/call`) before it pushes.
- The documentation site has a page on querying a folder from an AI agent (`zorba --directory` and `zorba-mcp`).
