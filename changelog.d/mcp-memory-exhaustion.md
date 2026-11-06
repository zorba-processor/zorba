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
- After an allocation failed while the QName pool's hash table was being resized,
  entries not yet moved were lost: built-in functions and the variables of a catch
  clause were then not found for the rest of the process (`XPST0017` on
  `parse-xml`, `string-join`, `sum`; `XPST0008` on `$err:description`). The table
  now keeps its old contents when a resize cannot allocate, and an erase no longer
  fails when the shrink after it cannot.
- `zorba-mcp`: a line of input longer than the message limit is refused without
  reading it into memory.
- `zorba-mcp`: a notification is never answered, also when memory runs out while it
  is handled; before, an error with `"id": null` was sent for it. A request is
  answered with its own id when memory runs out after its id is read; before, a
  failure while the id was copied gave `"id": null`.
- Running out of memory while the parse tree of a query was released could end the
  process (`std::terminate`, status 3 in `zorba-mcp`): the queue of nodes waiting to
  be destroyed allocated.
- Running out of memory while the text of a query was copied for the compiler
  compiled the part copied so far: `123456789012345678` could run as
  `123456789012345`, a wrong answer reported as a success. The text is now copied
  whole or the call fails. The same holds for a streamable string that is made
  seekable or read whole, for `createBase64Binary` from a stream in the C++ API,
  and for the source of a `zorba-mcp` predefined query.
- Running out of memory while a query object was created, or while an expression
  was recorded for the compiler, left memory behind.
