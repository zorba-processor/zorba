import module namespace m = "http://www.example.com/general-probe-reinit" at
                             "general_probe_reinit.xqlib";

m:init();

fn:string-join(
  for $index in (xs:QName("m:Range"), xs:QName("m:Equality"))
  let $n := fn:local-name-from-QName($index)
  return (
    m:point(fn:concat($n, " decimal keys"), $index, (0.1, 0.5)),
    m:point(fn:concat($n, " decimal then double key"), $index, (0.1, 1e10)),
    m:point(fn:concat($n, " untyped then typed key"), $index,
            (xs:untypedAtomic("10.0"), 7)),
    m:point(fn:concat($n, " typed then untyped key"), $index,
            (7, xs:untypedAtomic("10.0"))),
    m:point(fn:concat($n, " untyped then double key"), $index,
            (xs:untypedAtomic("7.0"), 3e0))),
  "&#10;")
