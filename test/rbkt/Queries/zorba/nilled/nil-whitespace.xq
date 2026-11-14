import schema namespace t = "http://zorba.io/test/nilled" at "nilled.xsd";

(: xsi:nil is an xs:boolean, so whitespace around its value is collapsed :)
(
  nilled(validate { <t:n xsi:nil=" true "/> }),
  nilled(validate { <t:n xsi:nil="&#9;1&#10;"/> })
)
