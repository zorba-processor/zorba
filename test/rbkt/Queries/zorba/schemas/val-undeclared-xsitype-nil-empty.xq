import schema namespace t = "http://zorba.io/test/val-lax-subtree"
  at "val-lax-subtree.xsd";

(: xsi:nil does not apply to an element without a declaration that has an xsi:type :)
let $x := (validate strict { <t:any><x xmlns:xs="http://www.w3.org/2001/XMLSchema" xsi:type="xs:string" xsi:nil="true"/></t:any> })/x
return ($x instance of element(*, xs:string), nilled($x), data($x) eq "")
