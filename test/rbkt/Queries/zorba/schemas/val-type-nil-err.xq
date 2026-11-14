import schema namespace t = "http://zorba.io/test/val-type-content"
  at "val-type-content.xsd";

(: Without an element declaration the root is not nilled: "" is not an xs:integer :)
validate type xs:integer { <e xsi:nil="true"/> }
