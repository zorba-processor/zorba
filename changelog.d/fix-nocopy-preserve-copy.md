### Fixed

- At O1 and O2, with `declare copy-namespaces preserve, no-inherit`, copying a
  tree in which a constructor had placed content without a copy (the no-copy
  optimization), e.g. `copy $c := local:nest(3) modify () return $c`, raised
  the internal error `zerr:ZXQP0002` (`copymode.theNsPreserve == false`). Such
  a copy now works, and each copied element gets the in-scope namespaces it
  gets at O0: an element placed without a copy does not inherit the bindings
  of the tree that references it.
