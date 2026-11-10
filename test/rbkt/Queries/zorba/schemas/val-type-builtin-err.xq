import schema namespace t = "http://zorba.io/test/val-type-content"
  at "val-type-content.xsd";

(: The value is not an xs:integer :)
validate type xs:integer { <e>abc</e> }
