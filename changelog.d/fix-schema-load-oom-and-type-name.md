### Fixed

- Fix a schema import when an allocation fails while the schema loads: it was
  reported as the error `ZXQP0014` or `ZXQP0035` of that schema instead of
  running out of memory, and the schema parser kept up to about 160 KB per
  failed import.
- Fix `validate type` when validation raises an error: the copy of the type
  name was not freed.
