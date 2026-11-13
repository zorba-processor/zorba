import schema namespace t = "http://zorba.io/test/val-type-content"
  at "val-type-content.xsd";

(: xsi:nil is allowed on the root; "" is a valid xs:string :)
local-name(validate type xs:string { <e xsi:nil="true"/> })
