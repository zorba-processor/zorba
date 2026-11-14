import schema namespace t = "http://zorba.io/test/nilled" at "nilled.xsd";
declare revalidation lax;

(: Revalidation after an update that makes the element nil sets the nilled property :)
variable $p := validate { <t:p><t:c>1</t:c></t:p> };
(
  delete node $p/t:c/text(),
  insert node attribute xsi:nil { "true" } into $p/t:c
);
nilled($p/t:c)
