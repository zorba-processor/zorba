import schema namespace t = "http://zorba.io/test/val-type-content"
  at "val-type-content.xsd";

(: An element of a simple type has no child elements :)
validate type xs:integer { <e><c/></e> }
