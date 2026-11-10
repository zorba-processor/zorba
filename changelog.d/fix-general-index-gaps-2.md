### Fixed

- Fix general index probes that missed keys an untyped value is equal to as an
  `xs:boolean`: an untyped key such as `"1"` or `"true"` is now cast to
  `xs:boolean`, so it meets `xs:boolean` probes, and an untyped probe meets
  `xs:boolean` keys, in point and range probes.
- Fix general range index probes without bounds, which did not select the
  domain nodes whose key is the empty sequence.
