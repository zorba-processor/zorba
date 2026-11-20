import schema namespace t = "http://zorba.io/test/nilled" at "nilled.xsd";

(: Lax validation of a declared element applies the same rule :)
validate lax { <t:m xsi:nil="true">x</t:m> }
