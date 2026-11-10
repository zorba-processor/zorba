### Breaking changes

- `fetch:content` and `fetch:content-binary` are now declared
  nondeterministic. A user function that calls one of them must be declared
  `%an:nondeterministic`; otherwise compiling a query that calls the user
  function raises `zerr:ZXQP0040`.

### Fixed

- Fix `fetch:content` and `fetch:content-binary` calling the URL resolver
  function item of a query prepared with `zq:prepare-main-module#3` or loaded
  with `zq:load-from-query-plan#3` fewer times than written: the optimizer
  treated both functions as deterministic and moved a call out of a loop.
  Every call now calls the resolver. `fn:doc` stays stable: calls with the
  same URI return the same document.
