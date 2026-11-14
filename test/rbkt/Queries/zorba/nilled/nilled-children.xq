import schema namespace t = "http://zorba.io/test/nilled" at "nilled.xsd";

(: The nil state of an element does not reach its children or its siblings :)
let $p := validate { <t:pn><t:c xsi:nil="true"/><t:e>1</t:e><t:pn xsi:nil="true"/></t:pn> }
return (
  nilled(validate { <t:pn xsi:nil="true"/> }),
  nilled($p),
  nilled($p/t:c),
  nilled($p/t:e),
  nilled($p/t:pn),
  nilled(validate lax { <t:m xsi:nil="true"/> })
)
