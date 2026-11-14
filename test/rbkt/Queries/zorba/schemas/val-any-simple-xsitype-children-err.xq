import schema namespace t = "http://zorba.io/test/val-lax-declared"
  at "val-lax-declared.xsd";

(: An element with a simple xsi:type has no element children, also when its declaration has no type :)
validate { <t:u xsi:type="xs:integer"><t:n>1</t:n></t:u> }
