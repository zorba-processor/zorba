import schema namespace t = "http://zorba.io/test/val-lax-declared"
  at "val-lax-declared.xsd";

(: An xsi:type on a declared element must derive from the declared type in lax validation too :)
validate lax { <t:k xsi:type="xs:integer"/> }
