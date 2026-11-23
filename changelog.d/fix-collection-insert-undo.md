### Fixed

- Fix a failed insert into a collection ending the process ("Unexpected error
  during pul undo"): when memory ran out while a node was attached to the
  collection, the node stayed in it unattached, and the undo of the nodes
  inserted before did not find them last. The collection now drops that node.
