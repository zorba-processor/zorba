declare namespace err = "http://www.w3.org/2005/xqt-errors";
(: UTF-7 has no direct form for '~': the conversion of the input fails part
 : way through the fragment, after some elements were started, and libxml2
 : stops the parser. The error is fn:parse-xml-fragment's own, not an
 : internal one. (A converter that accepts '~' parses the fragment.) :)
let $frag := concat('<?xml encoding="UTF-7"?><a>',
                    string-join(for $i in 1 to 3000 return '<b/>', ''),
                    '<c>~</c></a>')
return
  try { exists(fn:parse-xml-fragment($frag)) }
  catch err:FODC0006 { not(contains($err:description, "assertion failed")) }
