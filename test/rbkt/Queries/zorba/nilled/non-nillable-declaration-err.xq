import schema namespace t = "http://zorba.io/test/nilled" at "nilled.xsd";

(: xsi:nil true is not allowed against a non-nillable declaration :)
validate { <t:s xsi:nil="true"/> }
