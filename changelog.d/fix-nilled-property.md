### Fixed

- `fn:nilled` and the `nilled` property follow XDM: an element is nilled only
  if validation found `xsi:nil` true and the element declaration that governs
  it is nillable. Before, any validated element with `xsi:nil="true"` and no
  element or text children was nilled, including the result of
  `validate type xs:string { <e xsi:nil="true"/> }` and elements validated
  laxly without a declaration. Copies that preserve the type annotation keep
  the property; revalidation after an update and `schema:validate-in-place`
  set it.
- Validation treats `xsi:nil="1"` like `xsi:nil="true"`.
- Revalidation after an update that makes a simple-typed element nil (deletes
  its text and inserts `xsi:nil="true"`) no longer aborts the process.
