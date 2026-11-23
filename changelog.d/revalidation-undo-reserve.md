### Fixed

- Running out of memory while an integrity constraint is activated
  (`ic_ddl:activate`) is now the memory error, with the statement undone. It
  ended the process (`ZORBA_FATAL`): activations were applied after the
  statement's updates were made final.
- Undoing a type change that took a node's entry out of its tree's type map
  (an attribute that revalidation, or an update, makes `xs:untypedAtomic`)
  puts the entry back without allocating, so it cannot fail. Undoing a
  deactivated integrity constraint puts back the same constraint without
  allocating.
