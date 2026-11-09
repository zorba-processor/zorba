import module namespace m = "http://www.example.com/general-decimal-probe" at
                             "general_decimal_probe.xqlib";

m:init-maintained();

fn:string-join((
  m:point("maintained decimal point", xs:QName("m:a"), xs:QName("m:Auto"), 0.1),
  m:point("maintained double point", xs:QName("m:a"), xs:QName("m:Auto"),
          0.1e0),
  m:range("maintained decimal lower", xs:QName("m:a"), xs:QName("m:Auto"),
          0.1, (), fn:true()),
  m:range("maintained no bounds", xs:QName("m:a"), xs:QName("m:Auto"),
          (), (), fn:true())),
  "&#10;")
