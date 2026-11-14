import schema namespace t = "http://zorba.io/test/nilled" at "nilled.xsd";
declare construction preserve;

(: A copy that keeps the type annotation keeps the nilled property :)
(
  nilled(<w>{ validate { <t:n xsi:nil="true"/> } }</w>/t:n),
  nilled(<w>{ validate type xs:string { <e xsi:nil="true"/> } }</w>/e)
)
