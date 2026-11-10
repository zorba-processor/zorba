import schema namespace t = "http://zorba.io/test/val-type-content"
  at "val-type-content.xsd";

(: The element t:w declares a complex type; the named simple type governs :)
data(validate type xs:integer { <t:w>5</t:w> })
