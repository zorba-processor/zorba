import schema namespace t = "http://zorba.io/test/val-type-content"
  at "val-type-content.xsd";

(: The simple content is not an xs:integer :)
validate type t:IntWithUnit { <e unit="m">abc</e> }
