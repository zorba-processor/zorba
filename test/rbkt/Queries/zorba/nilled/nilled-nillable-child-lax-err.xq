import schema namespace t = "http://zorba.io/test/nilled" at "nilled.xsd";

(: Lax validation of a declared element applies the same rule :)
validate lax { <t:pn xsi:nil="true"><t:c/></t:pn> }
