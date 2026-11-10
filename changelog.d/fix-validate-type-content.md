### Fixed

- Fix `validate type`: the content of the element was not checked against the
  named type, so an invalid value of a built-in atomic type (in the element,
  in a child element or in simple content) and missing or unexpected child
  elements raised no `XQDY0027`. The declaration of the element name no
  longer applies to the validated element: no default or fixed value and no
  nillable.
- Fix validation of an element that is not validated (laxly assessed) and has
  a child with `xsi:type`: it raised `XQDY0027` for valid content.
