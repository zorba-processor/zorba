import schema namespace t = "http://zorba.io/test/nilled" at "nilled.xsd";

(: A nilled element has no text or element children :)
validate { <t:m xsi:nil="true">x<t:mc/></t:m> }
