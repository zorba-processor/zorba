### Fixed

- Fix general equality index probes that missed keys an untyped value is
  equal to: an untyped value missed `xs:gYear` keys unless the index also had
  numeric keys (`"2001"`) or the value was not an integer (`"1998Z"`), and an
  untyped value that is also of another type missed `xs:base64Binary` keys, or
  `xs:hexBinary` and `xs:base64Binary` keys if it was not an integer
  (`"1E10"`, `"P10Y"`, `"12345678901234567890"`), in both directions.
- Fix general index probes on untyped keys with more than 18 fractional
  digits, which were cast to `xs:double` from an `xs:decimal` rounded to 18
  digits instead of from the string: `"0.00000000000000000001"` missed
  `1e-20` and matched `0e0`.
- Fix general range index value probes whose two bounds are out of order as
  given but equal once promoted to the type of the keys, which selected
  nothing: bounds `0.1000000000000000001` and `0.1` select the key `0.1e0`.
- Fix NaN keys in a general range index, which raised ZSTR0041 when a second
  key was inserted; a probe without bounds selects them and no other probe
  does. NaN keys deleted from a general equality index are now removed (they
  stayed listed by `idml:keys`), and two NaN keys in a unique general range
  index no longer raise ZSTR0041.
