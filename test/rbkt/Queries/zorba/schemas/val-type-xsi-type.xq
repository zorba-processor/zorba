import schema namespace t = "http://zorba.io/test/val-type-content"
  at "val-type-content.xsd";

(: A derived xsi:type governs; one that is not derived is not used :)
let $a := validate type xs:decimal {
  <e xmlns:xs="http://www.w3.org/2001/XMLSchema" xsi:type="xs:integer">2</e>
}
let $b := validate type xs:integer {
  <e xmlns:xs="http://www.w3.org/2001/XMLSchema" xsi:type="xs:string">5</e>
}
let $c := validate type t:WithInt {
  <w xmlns:t="http://zorba.io/test/val-type-content" xsi:type="t:WithIntExt"><t:n>1</t:n><t:m>2</t:m></w>
}
return (
  data($a) instance of xs:integer,
  data($b) instance of xs:integer,
  $c instance of element(*, t:WithIntExt), data($c/t:m)
)
