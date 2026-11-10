import schema namespace t = "http://zorba.io/test/val-type-content"
  at "val-type-content.xsd";

(: The same value is rejected through the global element t:i :)
validate strict { <t:i>abc</t:i> }
