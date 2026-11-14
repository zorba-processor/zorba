import schema namespace t = "http://zorba.io/test/val-lax-subtree"
  at "val-lax-subtree.xsd";

(: xsi:nil does not apply to an element without a declaration that has an xsi:type :)
let $x := (validate strict { <t:any><x xsi:type="xs:integer" xsi:nil="true">5</x></t:any> })/x
return ($x instance of element(*, xs:integer), nilled($x), data($x))
