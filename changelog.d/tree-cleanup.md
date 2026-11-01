### Removed

- Dead source files that were not part of any build target (old interactive `*_test.cpp` programs, the unfinished parse-tree DOT visitor, the pre-DBGP debugger client and event listener, `TupleItem`, `ResultIteratorChainer`, the unused MAPM mutex wrapper, the Windows Mobile project, 2007 design documents, obsolete scripts and the broken `check_links` target).

### Added

- Unit test `test/libunit/ordpath` for `OrdPath` (compress/decompress round trip, document order, `insertBefore`/`insertAfter`/`insertInto`, relative position); it replaces the unbuilt `test/test_ordpath.cpp` command-line tool.
