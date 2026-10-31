### Breaking changes

- Recursion deeper than the call depth limit raises `zerr:ZXQP0069` instead of `zerr:ZXQP0003`; the limit comes from `SecurityPolicy::setMaxCallDepth()` (default 1024) and also applies to `eval` (formerly a fixed 256). `Properties::setMaxUDFCallDepth()` is no longer read by the engine.
- `XQuery::setTimeout()` overrides the security policy timeout for one query (seconds; 0 = none; -1 = policy). It used to have no effect.

### Security

- Enforce the security policy timeout during evaluation: a deadline checked every 1024 iterator calls aborts the query with `zerr:ZXQP0068`, which `try`/`catch` cannot catch; nested plans (`eval`, function bodies) share the deadline. No timeout by default. Replaces the unused timeout thread.
- Deep recursion raises `zerr:ZXQP0069` instead of crashing: besides the call depth limit, a function call is refused when less than a third of the thread's stack is left (closing the plan afterwards recurses through all call levels again).

### Added

- `zorba --timeout <seconds>` aborts queries that run longer; `--max-udf-call-depth` sets the policy's call depth limit.
