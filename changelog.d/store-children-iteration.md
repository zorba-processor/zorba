### Fixed

- Fix iteration over the children or attributes of a node, the members of an
  array or the keys of an object while a sequential expression updates that
  item: the iteration read storage the update had freed or moved, and
  returned wrong items or crashed. It now returns the contents the item had
  when the iteration started, the same result as at optimization level O0.
