import schema namespace t = "http://zorba.io/test/val-type-content"
  at "val-type-content.xsd";

(: The value is not a t:IntT :)
validate type t:IntT { <e>abc</e> }
