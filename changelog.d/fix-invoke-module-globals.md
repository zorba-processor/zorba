### Fixed

- `StaticContext::invoke()` now initializes the global variables of modules imported with `StaticContext::loadProlog()` before calling the function, as a query compiled against the same static context does. Previously a function reading a module global failed or read an uninitialized value.
