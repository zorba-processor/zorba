### Fixed

- `validate` no longer takes time proportional to the square of a tree's
  depth: each element took its base URI from the tree, walking up to the root
  (10 000 levels: 0.38 s, now 0.18 s).
- Saving or loading a compiled plan with a tree too deep for the stack
  raises `zerr:ZXQP0072` instead of crashing.
- Freeing trees that hold each other through the nodes that constructors
  share without copying no longer recurses.
- A failed allocation while libxml2 grows its input buffer is a memory error,
  not a malformed document, with libxml2 versions that report it only as the
  thread's last error (2.9).
