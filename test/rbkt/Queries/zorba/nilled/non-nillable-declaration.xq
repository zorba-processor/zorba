import schema namespace t = "http://zorba.io/test/nilled" at "nilled.xsd";

(: Not nilled: a non-nillable declaration, and lax validation without a declaration :)
(
  nilled(validate { <t:s/> }),
  nilled(validate { <t:p><t:c>1</t:c><t:d/></t:p> }/t:d),
  nilled(validate lax { <u xsi:nil="true"/> })
)
