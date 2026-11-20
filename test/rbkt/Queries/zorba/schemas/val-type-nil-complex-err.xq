import schema namespace t = "http://zorba.io/test/val-type-content"
  at "val-type-content.xsd";

(: Without an element declaration the root is not nilled: n is required :)
validate type t:WithInt { <w xsi:nil="true"/> }
