import schema namespace t = "http://zorba.io/test/val-lax-declared"
  at "val-lax-declared.xsd";

(: Same rule in lax validation :)
validate lax { <t:u xmlns:xs="http://www.w3.org/2001/XMLSchema" xsi:type="xs:integer"><t:n>1</t:n></t:u> }
