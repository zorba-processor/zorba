import module namespace m = "http://www.example.com/general-numeric-promotion" at
                             "general_numeric_promotion.xqlib";

m:init();

fn:string-join(
  for $index in (xs:QName("m:Range"), xs:QName("m:Equality"))
  let $n := fn:local-name-from-QName($index)
  for $probe in (
    ["decimal on float key", 0.1],
    ["float on decimal keys", xs:float("0.1")],
    ["double", 0.1e0],
    ["integer on float key", 16777217],
    ["float on integer keys", xs:float("16777216")],
    ["float on decimal key", xs:float("16777218")],
    ["large integer on float key", 100000000001],
    ["float on large integer key", xs:float("1e11")],
    ["float equal to large integer key", xs:float("100000006144")],
    ["double on large integer keys", 1e11],
    ["decimal on untyped integer", 3.00000000000000000001],
    ["float beyond xs:long", xs:float("1e19")],
    ["untyped decimal", xs:untypedAtomic("0.1")],
    ["untyped integer", xs:untypedAtomic("16777217")])
  return m:point(fn:concat($n, " ", $probe(1)), xs:QName("m:c"), $index,
                 $probe(2)),
  "&#10;")
