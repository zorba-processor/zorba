import schema namespace t = "http://zorba.io/test/val-type-content"
  at "val-type-content.xsd";

(: The sibling t:q after the complex child t:p is checked as xs:integer :)
validate type t:Nested { <e><t:p><t:n>1</t:n></t:p><t:q>abc</t:q></e> }
