import schema namespace t = "http://zorba.io/test/val-type-content"
  at "val-type-content.xsd";

(: The declaration of t:d does not apply: the empty content is not an xs:integer :)
validate type xs:integer { <t:d/> }
