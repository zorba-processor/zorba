import schema namespace t = "http://zorba.io/test/val-undeclared-complex-xsitype"
  at "val-undeclared-complex-xsitype.xsd";

(: The children of an element without a declaration are validated against its complex xsi:type :)
let $x := (validate strict { <t:any><x xmlns:t="http://zorba.io/test/val-undeclared-complex-xsitype" xsi:type="t:pair"><t:a>1</t:a><t:b>2020-01-01</t:b></x></t:any> })/x
return ($x instance of element(*, t:pair), data($x/t:a) instance of xs:integer, data($x/t:b) instance of xs:date)
