import schema namespace t = "http://zorba.io/test/val-type-content"
  at "val-type-content.xsd";

(: The element t:w declares a complex type; the value is still checked as xs:integer :)
validate type xs:integer { <t:w>abc</t:w> }
