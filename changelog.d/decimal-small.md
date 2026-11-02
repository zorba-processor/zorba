### Changed

- Reduce the memory of `xs:decimal` items: a stored value takes one heap block
  sized to fit (40 B up to 16 significant digits) instead of two blocks of
  144 B together, which cuts the heap of a parsed document by 7% for the CSV
  and 2% for the JSON file of `bench/store`. Results do not change.
