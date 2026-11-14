### Fixed

- A descendant path step (`//`) whose stack of children iterators could not grow for lack of memory kept the node it was about to enter, and with it the whole document, allocated for the rest of the process.
- Loading a document with DTD validation kept libxml2's parser context and the partial document allocated when running out of memory while an error of the document was reported (seen with libxml2 before 2.13).
