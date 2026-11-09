import module namespace m = "http://www.example.com/general-decimal-probe" at
                             "general_decimal_probe.xqlib";

m:init();

fn:string-join((
  for $index in (xs:QName("m:Range"), xs:QName("m:Equality"))
  let $n := fn:local-name-from-QName($index)
  return (
    m:point(fn:concat($n, " decimal point"), $index, 0.1),
    m:point(fn:concat($n, " exact decimal point"), $index, 0.5),
    m:point(fn:concat($n, " colliding decimal point"), $index,
            0.5000000000000000000001),
    m:point(fn:concat($n, " large integer point"), $index, 100000000000),
    m:point(fn:concat($n, " double point"), $index, 0.1e0)),
  m:range("decimal lower", 0.1, ()),
  m:range("decimal upper", (), 0.1),
  m:range("decimal both", 0.1000000000000000000001, 0.5),
  m:range("double lower", 0.5e0, ()),
  m:range("double upper", (), 0.1e0),
  m:range("no bounds", (), ())),
  "&#10;")
