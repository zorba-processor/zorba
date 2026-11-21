import schema namespace t = "http://zorba.io/test/val-undeclared-complex-xsitype"
  at "val-undeclared-complex-xsitype.xsd";

(: A required child of the complex xsi:type is missing :)
validate strict { <t:any><x xmlns:t="http://zorba.io/test/val-undeclared-complex-xsitype" xsi:type="t:pair"><t:a>1</t:a></x></t:any> }
