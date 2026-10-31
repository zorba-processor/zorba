### Security

- Deeply nested queries fail to compile with `zerr:ZXQP0070` instead of overflowing the stack. The limit is the nesting depth of the query's syntax tree, set with `SecurityPolicy::setMaxExprDepth()` (default 1500; 0 = no limit other than the stack); it is also capped by the stack left on the compiling thread, so threads with small stacks and eval from deep recursion fail cleanly too. Parse trees are destroyed without recursion, so even queries far beyond the limit (or with a syntax error deep inside) no longer crash.

### Added

- `zorba --max-expr-depth <depth>` sets the policy's expression nesting limit.

### Changed

- The plan serialization format version is now 27 (the security policy in a serialized static context includes the expression nesting limit); plans saved by earlier versions must be recompiled.
