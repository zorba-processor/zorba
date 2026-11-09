import module namespace m = "http://www.example.com/general-numeric-promotion" at
                             "general_numeric_promotion.xqlib";

m:init-maintained();

variable $unique := m:unique("unique integer and equal decimal",
                             <e v="100000000001"/>, <e v="100000000001.0"/>);

fn:string-join((
  for $index in (xs:QName("m:AutoRange"), xs:QName("m:AutoEquality"))
  let $n := fn:local-name-from-QName($index)
  for $probe in (
    ["float on decimal keys", xs:float("0.1")],
    ["float on large integer key", xs:float("1e11")],
    ["double on large integer keys", 1e11],
    ["float beyond xs:long", xs:float("1e19")],
    ["decimal on untyped integer", 3.00000000000000000001])
  return m:point(fn:concat($n, " ", $probe(1)), xs:QName("m:a"), $index,
                 $probe(2)),
  m:range("AutoRange no bounds", xs:QName("m:a"), xs:QName("m:AutoRange"),
          (), (), true()),
  $unique),
  "&#10;")
