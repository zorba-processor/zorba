### Fixed

- Fix a schema import when an allocation fails while the schema loads: it was
  reported as the error `ZXQP0014` of that schema instead of running out of
  memory.
- Fix `validate type` when validation raises an error: the copy of the type
  name was not freed.
