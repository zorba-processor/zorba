### Fixed

- Fix, in builds with `ZORBA_FOR_ONE_THREAD_ONLY=OFF`, races between queries
  that share functions, compiled and run on several threads:
  - the inline functions in the variable initializers of a prolog loaded with
    `loadProlog()`, which every query compiled against its static context
    generates, kept the iterators and the query of whichever query generated
    them last (also wrong on one thread, once a second query was compiled);
    the optimizer of every such query also recomputed the types of the
    prolog's expressions and marked their calls;
  - a function item that `fn:function-lookup` returned generated its plan when
    first called, on whichever thread called it;
  - a function coercion wrote the name of the coerced function at run time;
  - the cache of a function (`%an:cache`, `%an:strictlydeterministic`, or the
    automatic cache of a deterministic function with atomic arguments and
    result) was used by every query that calls the function without a lock;
  - a query compiled against the static context of another query
    (`XQuery::getStaticContext()`) generated the functions of that query that
    its main expression does not call, and failed an assertion in debug
    builds; `getStaticContext()` and `clone()` now seal every function the
    query declares;
  - the modules shipped with Zorba (file, http-client, ftp-client, sleep,
    zorba-query) created their functions on first lookup;
  - an external function calling `Zorba::getInstance()` while its query held
    the store lock could deadlock with a thread starting the engine.

### Changed

- `XQuery::getStaticContext()` and the first `XQuery::clone()` of a query
  optimize and generate the plans of the functions the query declares but does
  not call, as `loadProlog()` does for a prolog.
- The documentation of `ExternalModule` states that, in a multithreaded build,
  a module registered in a shared static context and its functions are called
  from several threads.
