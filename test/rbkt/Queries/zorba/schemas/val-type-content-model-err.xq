import schema namespace t = "http://zorba.io/test/val-type-content"
  at "val-type-content.xsd";

(: The required child t:n is missing :)
validate type t:WithInt { <w/> }
