import module namespace m = "http://www.example.com/numeric-probe-range" at
                             "numeric_probe_range.xqlib";

m:init();

fn:string-join(
  (
    "double 2: "          || m:point(2e0),
    "double 1e300: "      || m:point(1e300),
    "double -1e300: "     || m:point(-1e300),
    "double INF: "        || m:point(xs:double("INF")),
    "double -INF: "       || m:point(xs:double("-INF")),
    "float INF: "         || m:point(xs:float("INF")),
    "long max: "          || m:point(xs:long("9223372036854775807")),
    "long min: "          || m:point(xs:long("-9223372036854775808")),
    "unsignedLong max: [" ||
      m:point-indexed(xs:unsignedLong("18446744073709551615")) || "]",
    "untyped 2: ["        || m:point-untyped(xs:untypedAtomic("2")) || "]",
    "untyped 1e300: ["    || m:point-untyped(xs:untypedAtomic("1e300")) || "]",
    "untyped -INF: ["     || m:point-untyped(xs:untypedAtomic("-INF")) || "]",
    "from double -2.5 finds the double key: " ||
      fn:contains(m:range-from-indexed(-2.5e0), "3"),
    "from double -1e300 finds the double key: " ||
      fn:contains(m:range-from-indexed(-1e300), "3"),
    "from double 1e300: " || m:range-from(1e300),
    "from double INF: "   || m:range-from(xs:double("INF"))
  ),
  "&#10;")
