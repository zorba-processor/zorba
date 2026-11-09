### Fixed

- Fix general indexes, including the temporary index of a hash join, aborting
  on an `xs:dateTimeStamp` probe value and raising `ZDTY0012` on an
  `xs:dateTimeStamp` key; such values now compare with `xs:dateTime` keys and
  probe values as `xs:dateTime` values.
- Fix general indexes aborting on a `jn:null()` probe value or range bound
  and raising `ZDTY0012` on a `jn:null()` key. A null key is selected by a
  null probe value, by a range with a typed upper bound and no lower bound,
  and by inclusive null bounds; a null lower bound selects every other key.
  An untyped probe value or bound does not select null keys, where the
  comparison without the index raises `XPTY0004`.
