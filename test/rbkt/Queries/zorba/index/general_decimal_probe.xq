import module namespace m = "http://www.example.com/general-decimal-probe" at
                             "general_decimal_probe.xqlib";

m:init();

variable $unique1 := m:unique("unique decimal then float",
                              <e id="1" v="0.1"/>, <e id="2" t="f" v="0.1"/>);
variable $unique2 := m:unique("unique float then decimal",
                              <e id="1" t="f" v="0.1"/>, <e id="2" v="0.1"/>);
variable $unique3 := m:unique("unique decimals sharing an image",
                              <e id="1" v="0.1"/>,
                              <e id="2" v="0.1000000000000000000001"/>);
variable $unique4 := m:unique("unique decimal then double",
                              <e id="1" v="0.1"/>, <e id="2" t="d" v="0.1"/>);
variable $unique5 := m:unique("unique double then decimal of its float",
                              <e id="1" t="d" v="0.100000001490116119384765625"/>,
                              <e id="2" v="0.1"/>);
variable $unique6 := m:unique-reinsert();

fn:string-join((
  for $index in (xs:QName("m:Range"), xs:QName("m:Equality"))
  let $n := fn:local-name-from-QName($index)
  return (
    m:point(fn:concat($n, " decimal point"), $index, 0.1),
    m:point(fn:concat($n, " exact decimal point"), $index, 0.5),
    m:point(fn:concat($n, " colliding decimal point"), $index,
            0.5000000000000000000001),
    m:point(fn:concat($n, " untyped-equal decimal point"), $index,
            0.1000000000000000000002),
    m:point(fn:concat($n, " large integer point"), $index, 100000000000),
    m:point(fn:concat($n, " inexact integer point"), $index,
            9007199254740992),
    m:point(fn:concat($n, " inexact integer double point"), $index,
            9007199254740992e0),
    m:point(fn:concat($n, " double point"), $index, 0.1e0),
    m:point(fn:concat($n, " large decimal float point"), $index,
            xs:float("1e20")),
    m:point(fn:concat($n, " untyped point"), $index,
            xs:untypedAtomic("0.1000000000000000000001")),
    m:point(fn:concat($n, " untyped integer point"), $index,
            xs:untypedAtomic("7")),
    m:point(fn:concat($n, " decimal point on untyped integer"), $index, 7.0)),
  m:point("Range float point", xs:QName("m:Range"), xs:float("0.1")),
  m:range("decimal lower", 0.1, ()),
  m:range("decimal upper", (), 0.1),
  m:range("decimal both", 0.1000000000000000000001, 0.5),
  m:range("double both", 0.1e0, 0.5e0),
  m:range("double upper", (), 0.1e0),
  m:range("float both", xs:float("0.1"), xs:float("0.5")),
  m:range("untyped upper", (), xs:untypedAtomic("0.1000000000000000000001")),
  m:range("no bounds", (), ()),
  m:range("exclusive decimal lower", xs:QName("m:c"), xs:QName("m:Range"),
          0.1, (), fn:false()),
  m:range("exclusive decimal upper", xs:QName("m:c"), xs:QName("m:Range"),
          (), 0.1, fn:false()),
  m:range("exclusive double upper", xs:QName("m:c"), xs:QName("m:Range"),
          (), 0.1e0, fn:false()),
  m:range("exclusive float both", xs:QName("m:c"), xs:QName("m:Range"),
          xs:float("0.1"), xs:float("0.5"), fn:false()),
  m:point("maintained decimal point", xs:QName("m:a"), xs:QName("m:Auto"), 0.1),
  m:point("maintained double point", xs:QName("m:a"), xs:QName("m:Auto"),
          0.1e0),
  m:range("maintained decimal lower", xs:QName("m:a"), xs:QName("m:Auto"),
          0.1, (), fn:true()),
  m:range("maintained no bounds", xs:QName("m:a"), xs:QName("m:Auto"),
          (), (), fn:true()),
  $unique1, $unique2, $unique3, $unique4, $unique5, $unique6,
  m:keys()),
  "&#10;")
