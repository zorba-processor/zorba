(: Under the codepoint collation, each join selects what its comparison
   selects without an index: values that differ only in case are not equal. :)
declare default collation "http://www.w3.org/2005/xpath-functions/collation/codepoint";

let $doc := <d><a>x</a><b>X</b><b>y</b></d>
return fn:string-join((
  fn:concat("untyped eq untyped: ",
    fn:count(for $a in $doc/a
             for $b in $doc/b
             where xs:untypedAtomic($b) eq xs:untypedAtomic($a)
             return $b),
    " ", xs:untypedAtomic("X") eq xs:untypedAtomic("x")),
  fn:concat("untyped = untyped: ",
    fn:count(for $a in $doc/a
             for $b in $doc/b
             where xs:untypedAtomic($b) = xs:untypedAtomic($a)
             return $b),
    " ", xs:untypedAtomic("X") = xs:untypedAtomic("x")),
  fn:concat("string eq untyped: ",
    fn:count(for $a in $doc/a
             for $b in $doc/b
             where xs:string($b) eq xs:untypedAtomic($a)
             return $b),
    " ", "X" eq xs:untypedAtomic("x")),
  fn:concat("untyped eq string: ",
    fn:count(for $a in $doc/a
             for $b in $doc/b
             where xs:untypedAtomic($b) eq xs:string($a)
             return $b),
    " ", xs:untypedAtomic("X") eq "x"),
  fn:concat("anyURI eq anyURI: ",
    fn:count(for $a in $doc/a
             for $b in $doc/b
             where xs:anyURI($b) eq xs:anyURI($a)
             return $b),
    " ", xs:anyURI("X") eq xs:anyURI("x")),
  fn:concat("anyURI = anyURI: ",
    fn:count(for $a in $doc/a
             for $b in $doc/b
             where xs:anyURI($b) = xs:anyURI($a)
             return $b),
    " ", xs:anyURI("X") = xs:anyURI("x")),
  fn:concat("string eq anyURI: ",
    fn:count(for $a in $doc/a
             for $b in $doc/b
             where xs:string($b) eq xs:anyURI($a)
             return $b),
    " ", "X" eq xs:anyURI("x")),
  fn:concat("anyURI eq string: ",
    fn:count(for $a in $doc/a
             for $b in $doc/b
             where xs:anyURI($b) eq xs:string($a)
             return $b),
    " ", xs:anyURI("X") eq "x"),
  fn:concat("anyURI = untyped: ",
    fn:count(for $a in $doc/a
             for $b in $doc/b
             where xs:anyURI($b) = xs:untypedAtomic($a)
             return $b),
    " ", xs:anyURI("X") = xs:untypedAtomic("x")),
  fn:concat("untyped = anyURI: ",
    fn:count(for $a in $doc/a
             for $b in $doc/b
             where xs:untypedAtomic($b) = xs:anyURI($a)
             return $b),
    " ", xs:untypedAtomic("X") = xs:anyURI("x")),
  fn:concat("distinct anyURI: ",
    fn:count(fn:distinct-values((xs:anyURI("x"), xs:anyURI("X"))))),
  fn:concat("anyURI lt anyURI: ",
    xs:anyURI("X") lt xs:anyURI("x"), " ", xs:anyURI("x") lt xs:anyURI("X"))),
  "&#10;")
