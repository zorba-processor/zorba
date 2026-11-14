import schema namespace t = "http://zorba.io/test/val-lax-subtree"
  at "val-lax-subtree.xsd";

(: xsi:nil does not apply to an element without a declaration that has an xsi:type :)
for $x in (validate lax { <x xmlns:xs="http://www.w3.org/2001/XMLSchema" xsi:type="xs:integer" xsi:nil="true">5</x> }, (validate lax { <w><x xmlns:xs="http://www.w3.org/2001/XMLSchema" xsi:type="xs:integer" xsi:nil="true">5</x></w> })/x)
return ($x instance of element(*, xs:integer), nilled($x), data($x))
