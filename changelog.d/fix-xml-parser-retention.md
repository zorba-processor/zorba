### Fixed

- A descendant path step (`//`) whose stack of children iterators could not grow for lack of memory kept the node it was about to enter, and with it the whole document, allocated for the rest of the process.
