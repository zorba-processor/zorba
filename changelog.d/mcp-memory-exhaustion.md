### Fixed

- `zorba-mcp`: a query that runs out of memory under `--max-memory` gets the tool
  error `memory_limit` and the same server answers the next call. Before, depending
  on which allocation failed, the server could end with no response: allocations in
  the JSON string lexers and CSV parser, in the release of JSON values, in the QName
  pool, and in MAPM's decimal numbers (which ended the process with status 100, also
  in `zorba`). If memory runs out where nothing can recover, for example while the
  server starts, `zorba-mcp` exits with status 3 and a message on stderr; `--help`
  lists what libraries do instead.
- Running out of memory while parsing XML, in casts, in regular expressions, in `file:`
  functions and in external functions is no longer a false result or an error that a
  catch clause can catch, such as the one in `dir:open`: it ends the query, as a
  timeout does.
- `zorba-mcp`: a line of input longer than the message limit is refused without
  reading it into memory; a large stack limit no longer uses up `--max-memory` at the
  start.
