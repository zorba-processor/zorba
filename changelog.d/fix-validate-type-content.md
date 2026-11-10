### Fixed

- Fix `validate type`: the content of the element was not checked against the
  named type, so an invalid value of a built-in atomic type (in the element,
  in a child element or in simple content) and missing or unexpected child
  elements raised no `XQDY0027`.
