### Fixed

- Inserting a node whose new node id exceeds the size limit (a node about a
  thousand levels deep) into a tree raised the node id error after the node
  had been moved half into the target tree: the query then crashed while it
  closed. The new ids are now computed before the node is moved, so the error
  leaves both trees as they were.
