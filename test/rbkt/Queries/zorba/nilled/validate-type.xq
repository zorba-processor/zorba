import schema namespace t = "http://zorba.io/test/nilled" at "nilled.xsd";

(: No element declaration governs the element, so xsi:nil does not make it nilled :)
nilled(validate type xs:string { <e xsi:nil="true"/> })
