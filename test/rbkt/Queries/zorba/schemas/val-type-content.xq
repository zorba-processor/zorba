import schema namespace t = "http://zorba.io/test/val-type-content"
  at "val-type-content.xsd";

let $a := validate type xs:integer { <e>42</e> }
let $b := validate type t:IntT { <e>7</e> }
let $c := validate type t:WithInt { <w><t:n>5</t:n></w> }
let $d := validate type t:IntWithUnit { <e unit="m">3</e> }
return (
  data($a) instance of xs:integer, data($a),
  data($b) instance of t:IntT, data($b),
  data($c/t:n) instance of xs:integer, data($c/t:n),
  data($d) instance of xs:integer, data($d)
)
