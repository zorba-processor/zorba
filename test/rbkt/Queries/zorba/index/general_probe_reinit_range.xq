import module namespace m = "http://www.example.com/general-probe-reinit" at
                             "general_probe_reinit.xqlib";

m:init();

fn:string-join((
  m:range("untyped then typed lower", (xs:untypedAtomic("8"), 3), ()),
  m:range("typed then untyped lower", (3, xs:untypedAtomic("8")), ()),
  m:range("untyped then typed upper", (), (xs:untypedAtomic("05"), 8)),
  m:range("typed then untyped upper", (), (8, xs:untypedAtomic("05"))),
  m:range("untyped then typed lower, upper", (xs:untypedAtomic("8"), 3), 8),
  m:range("lower, untyped then typed upper", 3, (xs:untypedAtomic("05"), 8))),
  "&#10;")
