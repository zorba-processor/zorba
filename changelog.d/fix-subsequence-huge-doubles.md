### Fixed

- Fix `fn:subsequence` with huge or non-finite start and length arguments such
  as `1e300`, `-INF` or `NaN`, and with integer arguments at the limits of
  `xs:long`. Positions are now compared as the specification defines, so
  `subsequence((1,2,3), 1e300)` is empty and `subsequence((1,2,3), -1e300)`
  returns all items on every platform.
