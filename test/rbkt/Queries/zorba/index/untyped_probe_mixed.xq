import module namespace m = "http://www.example.com/untyped-probe-mixed" at
                             "untyped_probe_mixed.xqlib";

m:init();

fn:concat("range: ", m:range-from(xs:untypedAtomic("2000-01-01")),
          "; point: ", m:point(xs:untypedAtomic("2001-06-01")))
