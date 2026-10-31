### Added

- Debugger server: DBGP `property_value` returns the full value of a variable in the top stack frame (`-m` limits the bytes sent, `-p` selects a page of 32 sequence items).
- Debugger server: DBGP `property_set` assigns the result of an XQuery expression to an assignable prolog or block-local variable of the suspended query. Unknown and non-assignable variables return error 300; invalid expressions and values that do not match the declared type return error 206.

### Fixed

- Debugger: breakpoints, stepping and eval never fired because the debug-mode wrapping of expressions was discarded by the translator.
- Debugger: every eval of a variable failed with XPDY0002 because the context item was passed to the debugger eval under its old internal name.
- Eval queries run inside a debugged query are no longer compiled in debug mode.
- Executing a query compiled in debug mode without a debugger session no longer crashes.
- Debugger client: `property_value` sent a `property_get` command.
- Debugger server: error messages are XML-escaped in DBGP error responses.
- The `debug_iter_serialization` unit test was never built or run.
