import module namespace m = "http://www.example.com/general-index-probe-gaps" at
                             "general_index_probe_gaps.xqlib";

m:create(xs:QName("m:p"),
         (<e id="1" v="0.00000000000000000001"/>, <e id="2" v="0"/>),
         (xs:QName("m:PEquality"), xs:QName("m:PRange")));

(: Compared with a number, an untyped value is cast to xs:double from its
   string value. :)
fn:string-join((
  for $index in (xs:QName("m:PEquality"), xs:QName("m:PRange"))
  let $n := fn:local-name-from-QName($index)
  for $probe in (["1e-20", 1e-20], ["0e0", 0e0], ["decimal 0", 0.0])
  return m:point(fn:concat($n, " ", $probe(1)), xs:QName("m:p"), $index,
                 $probe(2)),
  m:range("PRange lower 1e-20", xs:QName("m:p"), xs:QName("m:PRange"),
          1e-20, ()),
  m:range("PRange upper 0e0", xs:QName("m:p"), xs:QName("m:PRange"),
          (), 0e0)),
  "&#10;")
