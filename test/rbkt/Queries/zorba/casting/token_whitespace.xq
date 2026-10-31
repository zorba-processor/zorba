(: xs:token collapses whitespace (XSD 1.1 Part 2, 3.4.2; F&O 3.0 19.1.5). :)
(concat("[", ("  a &#x9; b&#xA;  ") cast as xs:token, "]"),
 concat("[", (" a  b ") cast as xs:token, "]"),
 concat("[", ("&#xA;&#x9;") cast as xs:token, "]"))
