### Changed

- Share one store item per xs:integer value from -128 to 1023, as the boolean
  items are shared, instead of allocating an item for every value. Parsing the
  store benchmark's 100 MiB files takes 169 MiB less heap for JSON and 59 MiB
  less for CSV; the savings depend on how many of a document's integers fall in
  the range. Each store holds about 64 KB more for the table.
