### Fixed

- `dir:documents` no longer keeps every parsed document in memory until the query ends: it sorted the records with an `order by` over the loaded documents, which holds the result of every tuple until the last one is loaded. It now sorts the paths first and parses each document when the caller asks for it, so a query that consumes the documents one at a time holds one of them (peak RSS of a scan of 1000 files of 200 KB: 4610 MiB before, 55 MiB after).
