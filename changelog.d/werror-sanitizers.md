### Fixed

- Fix `pop_nodestack` in the translator returning an uninitialized pointer
  when asked to pop zero entries; it now returns null.
- Fix two debug assertions in the QName pool free list that assigned instead
  of comparing, so they checked nothing.
- Fix the range check for converting a floating-point value to a signed
  integer type accepting a value one past the maximum (for example
  2147483648.0 for `xs:int` from `xs:float`), which then overflowed in the
  conversion.
