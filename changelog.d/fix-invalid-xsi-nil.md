### Fixed

- Fix validation accepting an `xsi:nil` value that is not an `xs:boolean`
  (`"yes"`, `"TRUE"`): such an element was taken as not nilled; validation now
  fails with XQDY0027.
