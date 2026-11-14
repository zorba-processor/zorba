### Fixed

- A general probe with a numeric search key on an index without a key type,
  or with `xs:untypedAtomic*`, raises `err:FORG0001` when an untyped key is
  not castable to `xs:double`, as `=` does. It returned the other matches.
- An index whose key type is `xs:float` accepts an `xs:double` search key and
  compares the keys promoted to `xs:double`, as the comparison does. It raised
  `err:XPTY0004`.
- In an index whose key type is `xs:double`, an untyped search key raises
  `err:XPTY0004` in value probes, as `eq` does, and is cast to `xs:double` in
  general probes, as `=` does. It matched nothing.
- The value range probes of a general index whose key type is `xs:duration`
  raise `err:XPTY0004` for duration bounds that have no order with a key, as
  `ge` and `le` do. They returned the keys in range.
