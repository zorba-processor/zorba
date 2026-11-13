import schema namespace t = "http://zorba.io/test/nilled" at "nilled.xsd";
declare construction strip;

(: A copy that strips the type annotation is not nilled :)
nilled(<w>{ validate { <t:n xsi:nil="true"/> } }</w>/t:n)
