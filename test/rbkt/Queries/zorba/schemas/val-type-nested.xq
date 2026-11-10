import schema namespace t = "http://zorba.io/test/val-type-content"
  at "val-type-content.xsd";

let $v := validate type t:Nested { <e><t:p><t:n>1</t:n></t:p><t:q>2</t:q></e> }
return (
  data($v/t:p/t:n) instance of xs:integer, data($v/t:p/t:n),
  data($v/t:q) instance of xs:integer, data($v/t:q)
)
