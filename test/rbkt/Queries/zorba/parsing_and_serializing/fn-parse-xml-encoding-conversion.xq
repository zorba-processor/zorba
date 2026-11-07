declare namespace err = "http://www.w3.org/2005/xqt-errors";
(: UTF-7 has no direct form for '~': the conversion of the input fails part
 : way through the document, after some elements were started, and libxml2
 : stops the parser. The error is fn:parse-xml's own, not an internal one.
 : (A converter that accepts '~' parses the document.) The input is more
 : than one packet of the loader (8192 bytes), so the parser has started
 : when the conversion fails. :)
let $doc := concat('<?xml version="1.0" encoding="UTF-7"?><a>',
                   string-join(for $i in 1 to 3000 return '<b/>', ''),
                   '<c>~</c></a>')
return
  try { count(fn:parse-xml($doc)/a/*) eq 3001 }
  catch err:FODC0006 { not(contains($err:description, "assertion failed")) }
