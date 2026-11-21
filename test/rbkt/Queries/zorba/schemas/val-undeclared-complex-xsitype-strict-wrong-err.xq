import schema namespace t = "http://zorba.io/test/val-undeclared-complex-xsitype"
  at "val-undeclared-complex-xsitype.xsd";

(: A child the content model of the complex xsi:type does not allow :)
validate strict { <t:any><x xmlns:t="http://zorba.io/test/val-undeclared-complex-xsitype" xsi:type="t:pair"><t:a>1</t:a><t:c>2020-01-01</t:c></x></t:any> }
