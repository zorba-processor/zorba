### Fixed

- `zorba-mcp` document cache: a query gets its own copy of a kept document, so its results are those of reading the file. Before, a kept document had one node identity for all reads, and its place in the order of documents came from when it was kept, so path expressions and unions over several documents could return another order, and two reads of a file counted as one.
- `zorba-mcp` document cache: a kept XML or HTML document has the base URI of a document read from the file.
- `zorba-mcp` document cache: a file changed in the last 20 ms is not kept yet. A write right after such a read could carry the same timestamps, and the old content stayed in the cache.
- `zorba-mcp` document cache: with room for them, all files enter. Two files could take the same slot of the table that counts first reads and never enter; about 2 in 100 files did at 100 files. The table keeps its size, 32 KiB.
- `zorba-mcp` document cache: a file read through another spelling of its root shares the record of that root.
- A copy of a document with a generated document URI gets its own generated URI.
