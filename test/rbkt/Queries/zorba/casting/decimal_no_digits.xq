declare namespace err = "http://www.w3.org/2005/xqt-errors";
(: The xs:decimal and xs:integer lexical spaces require at least one digit
   (XSD 1.1 Part 2, 3.3.3, 3.4.13; F&O 3.0 19.1.1.1). :)
(for $s in ("&#x9;&#xA;&#xD;", " ", "+", "-", ".", "+.")
 return try { string(xs:decimal($s)) } catch * { local-name-from-QName($err:code) },
 for $s in (" ", "+", "-")
 return try { string(xs:integer($s)) } catch * { local-name-from-QName($err:code) },
 string(xs:decimal(" 1. ")), string(xs:decimal(".5")))
