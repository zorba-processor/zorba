### Fixed

- `validate type` no longer treats `xsi:nil="true"` on the validated element as
  making it nilled: without an element declaration the attribute is ignored and
  the content is validated against the type, so
  `validate type xs:integer { <e xsi:nil="true"/> }` raises `XQDY0027`.
- `validate type` uses an `xsi:type` on the validated element when it names a
  type derived from the named type: the content is validated against it and
  the element is annotated with it. Before, the attribute was ignored. An
  `xsi:type` that names a type not derived from the named type is still
  ignored.
