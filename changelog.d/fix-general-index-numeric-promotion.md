### Fixed

- Fix general index probes that compare numbers of different types, which did
  not promote them as a general comparison does: an `xs:decimal` value missed
  an equal `xs:float` key (`0.1` and `xs:float("0.1")`), an `xs:float` value
  missed `xs:integer` keys that round to it (`16777217` for
  `xs:float(16777216)`) and on an equality index all but one `xs:decimal` key
  equal to it, an `xs:decimal` value missed an untyped key equal to it as
  `xs:double` (`3.00000000000000000001` and `"3"`), and range bounds on such
  keys were compared the same wrong ways.
- Fix general index probes with a NaN value or bound, which raised ZSTR0041
  instead of matching no number.
- Fix a crash when the last node with a given key is deleted from a
  collection with an automatic general equality index (the key was released
  before its entry was removed).
