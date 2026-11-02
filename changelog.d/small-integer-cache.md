### Changed

- Share one store item per xs:integer value from -128 to 1023, as the boolean
  items are shared, instead of allocating an item for every value. Parsing the
  store benchmark's 100 MiB files takes 8.0% less heap for JSON (2100 to 1932
  MiB) and 3.5% less for CSV (1656 to 1597 MiB); the savings depend on how many
  of a document's integers fall in the range. Each store holds about 64 KB more
  for the table.
