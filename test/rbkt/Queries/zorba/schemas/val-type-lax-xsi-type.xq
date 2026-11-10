import schema namespace t = "http://zorba.io/test/val-type-content"
  at "val-type-content.xsd";

(: x is not validated; its child y has an xsi:type :)
local-name(validate type t:LaxAny {
  <r><x><y xmlns:xs="http://www.w3.org/2001/XMLSchema"
           xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
           xsi:type="xs:integer">1</y></x></r>
})
