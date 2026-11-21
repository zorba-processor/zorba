import schema namespace t = "http://zorba.io/test/nilled" at "nilled.xsd";

(: A nilled element has no element children :)
validate { <t:pn xsi:nil="true"><t:e>1</t:e></t:pn> }
