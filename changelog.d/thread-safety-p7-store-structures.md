### Fixed

- Fix, in builds with `ZORBA_FOR_ONE_THREAD_ONLY=OFF`, crashes and corrupted
  results when several threads read the children or attributes of nodes, or
  the members of arrays and objects, at the same time (for example documents
  loaded with `fn:doc`): the store kept the open iterations of all threads in
  one unsynchronized table. Each thread now has its own.
- Fix, in builds with `ZORBA_FOR_ONE_THREAD_ONLY=OFF`, listing the available
  collections, indexes, maps or documents while another thread adds or
  removes one: the names are now copied under the store map's lock.
- Fix, in builds with `ZORBA_FOR_ONE_THREAD_ONLY=OFF`, a collection iterator
  destroyed while open releasing a collection lock it never took.
- Fix the hash of node-reference URIs (structural `xs:anyURI` items), which
  was the hash of the empty string until the URI's text was first read; the
  text is now computed when the item is created, so it is also safe to read
  from several threads.
