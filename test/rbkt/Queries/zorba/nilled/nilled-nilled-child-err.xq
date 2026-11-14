import schema namespace t = "http://zorba.io/test/nilled" at "nilled.xsd";

(: A nilled element has no element children, a nilled one included :)
validate { <t:pn xsi:nil="true"><t:pn xsi:nil="true"/></t:pn> }
