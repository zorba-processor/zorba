### Fixed

- Fix the reference of a JSON object or array staying registered after
  `dml:delete`, `dml:delete-first`, `dml:delete-last` or `dml:truncate`
  removed the item from its collection and the item was freed:
  `ref:dereference` read freed memory, and the store stopped at shutdown with
  "node references still in the nodes to references map". Dereferencing the
  reference of a freed item now returns the empty sequence, as for nodes.
