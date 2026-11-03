### Fixed

- `zorba-mcp`: a query that runs out of memory under `--max-memory` gets the tool
  error `memory_limit` and the same server answers the next call. Before, depending
  on which allocation failed, the server could end with no response: allocations in
  the JSON string lexers and CSV parser, in the release of JSON values, and in
  MAPM's decimal numbers (which ended the process with status 100, also in `zorba`).
  If memory runs out where nothing can recover, for example while the server starts,
  `zorba-mcp` exits with status 3 and a message on stderr.
- Running out of memory while parsing XML, or in `file:` functions and external
  functions, is no longer an error that a catch clause can catch, such as the one
  in `dir:open`: it ends the query, as a timeout does.
