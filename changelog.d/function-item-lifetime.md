### Fixed

- C++ API: a function item taken from a query result stays usable after the query is closed or destroyed. Invoking it with `StaticContext::invoke()` or passing it to another query used freed memory; the creating query's compiled functions and contexts are now released when the last such function item goes away (cycles through prolog variables holding function items are collected). The same applies to function items created by `reflection:eval` (which were freed when the eval was re-run), to function items referring to functions declared by `StaticContext::loadProlog()`, and to partially applied function items, whose closure could be freed with the original function item.
- A function item reads the prolog variables of the query that created it, as XQuery 3.0 requires, also when it is invoked by another query; it read the invoking query's variables (wrong values or XPDY0002, including when the invoking query declared a variable with the same name).
- `%an:cache` and `%an:strictlydeterministic` functions no longer cache calls whose arguments or results contain function items.
- A clone of a query (`XQuery::clone()`) no longer uses freed memory when the original query is closed first.
- The functions declared by `StaticContext::loadProlog()` no longer report compile-time warnings to freed memory.
