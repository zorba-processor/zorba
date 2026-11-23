declare namespace err = "http://www.w3.org/2005/xqt-errors";
(: A file is decoded by its declared encoding. UTF-7 has no direct form for
 : '~': the conversion fails part way through the document, after some
 : elements were started, and libxml2 stops the parser. The error is
 : fn:doc's own, not an internal one. (A converter that accepts '~' parses
 : the document.) The file is more than one packet of the loader (8192
 : bytes), so the parser has started when the conversion fails. :)
try { count(fn:doc("encoding-conversion-utf7.xml")/a/*) eq 3001 }
catch err:FODC0002 { not(contains($err:description, "assertion failed")) }
