### Added

- Debugger server: DBGP `property_value` returns the full value of a variable in the top stack frame (`-m` limits the bytes sent, `-p` selects a page of 32 sequence items).
- Debugger server: DBGP `property_set` assigns the result of an XQuery expression to an assignable prolog or block-local variable of the suspended query; the value must match the declared type as in an assignment. An existing variable that can not take the value answers `success="0"`, an unknown variable error 300, and an expression that fails to evaluate error 206.
- Debugger server: DBGP `property_get` honours `-p` (pages of 32 children, with `page`, `pagesize` and `numchildren`) and `-m` (truncated value, with `size`).
- Debugger client API: `DebuggerClient::property_set` overload that sends the value; xqdb command `set -n <name> -v <expression>`.

### Fixed

- Debugger: breakpoints, stepping and eval never fired because the debug-mode wrapping of expressions was discarded by the translator.
- Debugger: every eval of a variable failed with XPDY0002 because the context item was passed to the debugger eval under its old internal name.
- Eval queries run inside a debugged query are no longer compiled in debug mode.
- Executing a query compiled in debug mode without a debugger session no longer crashes.
- Debugger client: `property_value` sent a `property_get` command.
- Debugger server: error messages are XML-escaped in DBGP error responses.
- The `debug_iter_serialization` unit test was never built or run.
