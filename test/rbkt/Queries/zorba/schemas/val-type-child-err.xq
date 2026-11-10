import schema namespace t = "http://zorba.io/test/val-type-content"
  at "val-type-content.xsd";

(: The value of the child t:n is not an xs:integer :)
validate type t:WithInt { <w><t:n>abc</t:n></w> }
