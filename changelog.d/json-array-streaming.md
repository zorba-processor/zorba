### Added

- Add the option `zorba-streaming-members` of `parse-json`. With it,
  `parse-json($text, { "zorba-streaming-members" : true })[]` reads the members of
  a top-level array one at a time, without building the array, so that peak memory
  follows the largest member, plus the text unless it is read directly with
  `file:read-text` in a query where nothing can write while it runs: no sequential
  expression, no update, no dynamic function call, no eval or invoke, no call of a
  function that takes a function as argument, and no function item of a sequential
  or updating function, in the query or in a function it calls, both in the query
  that compiles the call and in the query that runs it. The check trusts the
  declarations: an external function that writes but is not declared sequential
  is not seen. At optimization level O0, the text of `file:read-text` called with
  one argument is held in memory. Each top-level array is parsed twice: first to
  check it, so that errors and their order stay the same, then to return its
  members. Without the option nothing changes.
