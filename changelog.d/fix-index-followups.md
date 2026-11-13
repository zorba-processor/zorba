### Fixed

- A general index whose key type is `xs:untypedAtomic*` accepts `xs:string`
  and `xs:anyURI` search keys and bounds, and its value probes compare an
  untyped search key or bound as a string, as a comparison with an untyped
  value does. They raised `err:XPTY0004`, and an untyped search key in a value
  probe failed an internal assertion.
- The value probes of a general index without a key type
  (`probe-index-point-value`, `probe-index-range-value` and their `-skip`
  forms) raise `err:XPTY0004` when the index holds a key that a value
  comparison cannot compare with the search key or a bound, as the value
  comparison the probe stands for does. They returned the nodes whose keys
  could be compared and skipped the others, unless those keys were untyped
  values castable to a type other than `xs:string`.
