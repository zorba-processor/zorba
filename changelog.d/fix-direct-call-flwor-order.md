### Breaking changes

- A FLWOR expression with a direct call of a sequential function runs each iteration of a `for` clause complete before the next, as a call through a function item does: `for $i in 1 to 3 let $a := local:bump() let $b := local:bump() return ($a, $b)` gives `1 2 3 4 5 6` instead of `1 4 2 5 3 6`, at every optimization level. The later clauses and the return clause run per tuple, so a `let`, `where` or `order by` reads the state left by the earlier clauses of its own iteration. The domain of every `for` clause of such an expression gives its whole binding sequence first (XQuery Scripting Extension 1.0, 6.7), per tuple of the clauses before it; the `no-materialization` pragma keeps the domains lazy, as before.
- A positional predicate, `fn:head`, `fn:exists` or `fn:subsequence` over a loop with a direct sequential call stops after the items it needs, as over a loop with a call through a function item: `((for $i in 1 to 3 return local:bump())[1], $g)` gives `1 1` instead of `1 3`. A call of a sequential function written directly as their argument still runs to its end; the decision is taken on the query as written, before the optimizer.

### Changed

- At O1 and O2, an expression that reads an assignable variable, a nondeterministic function, `fn:doc`, `fn:collection` or a store function is no longer moved out of a sequential FLWOR expression.
