import module namespace m = "http://www.example.com/general-numeric-probe" at
                             "general_numeric_probe.xqlib";

m:init();

fn:string-join((
  m:range("double lower", 2.5e0, ()),
  m:range("double upper", (), 2.5e0),
  m:range("negative double lower", -2.5e0, ()),
  m:range("negative double upper", (), -2.5e0),
  m:range("double both", 1.5e0, 2.5e0),
  m:range("untyped lower", xs:untypedAtomic("2.00000000000000000001"), ()),
  m:range("untyped upper", (), xs:untypedAtomic("0.99999999999999999999")),
  m:range("untyped decimal lower", xs:untypedAtomic("1.5"), ()),
  m:point("untyped point", xs:untypedAtomic("2.00000000000000000001"))),
  "&#10;")
