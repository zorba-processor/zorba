### Added

- Debugger server: DBGP `property_value` returns the full value of a variable in the top stack frame (`-m` limits the bytes sent, `-p` selects a page of 32 sequence items).
- Debugger server: DBGP `property_set` assigns the result of an XQuery expression to an assignable prolog or block-local variable of the suspended query. Unknown and non-assignable variables return error 300; invalid expressions and values that do not match the declared type return error 206.

### Fixed

- Debugger client: `property_value` sent a `property_get` command.
- Debugger server: error messages are XML-escaped in DBGP error responses.
- The `debug_iter_serialization` unit test was never built or run.
