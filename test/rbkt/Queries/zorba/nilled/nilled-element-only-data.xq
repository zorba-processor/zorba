import schema namespace t = "http://zorba.io/test/nilled" at "nilled.xsd";

(: The typed value of a nilled element is the empty sequence, also when its
   type has element-only or mixed content :)
(
  count(data(validate { <t:pn xsi:nil="true"/> })),
  count(data(validate { <t:m xsi:nil="true"/> })),
  count(data((validate { <t:p><t:c xsi:nil="true"/></t:p> })/t:c))
)
