### Fixed

- `validate type` no longer treats `xsi:nil="true"` on the validated element as
  making it nilled: without an element declaration the attribute is ignored and
  the content is validated against the type, so
  `validate type xs:integer { <e xsi:nil="true"/> }` raises `XQDY0027`.
