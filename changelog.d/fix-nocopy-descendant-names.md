### Fixed

- At O1 and O2, a descendant step with a name test (`//a`,
  `descendant-or-self::a`) missed elements nested in an element of the same
  name when the inner element came from an expression whose result a
  constructor did not copy (the no-copy optimization), e.g. a recursive
  function `<a>{ local:nest($n - 1) }</a>`: `count(local:nest(3)//a)` gave 1
  instead of 2. Placing a subtree without copying it now marks the elements
  above it that a copy would mark as containing an element of their own name.
