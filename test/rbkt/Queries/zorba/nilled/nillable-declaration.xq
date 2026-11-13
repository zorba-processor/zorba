import schema namespace t = "http://zorba.io/test/nilled" at "nilled.xsd";

(: Nilled only with xsi:nil true against a nillable declaration :)
(
  nilled(validate { <t:n xsi:nil="true"/> }),
  nilled(validate { <t:n xsi:nil="1"/> }),
  nilled(validate { <t:n xsi:nil="false"/> }),
  nilled(validate { <t:n/> }),
  nilled((validate { <t:p><t:c xsi:nil="true"/></t:p> })/t:c),
  nilled(validate { <t:p><t:c xsi:nil="true"/></t:p> })
)
