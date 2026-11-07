declare namespace err = "http://www.w3.org/2005/xqt-errors";
(: The declared encoding does not cover a character of the document: the
 : parser stops part way through, after some elements were started. The
 : error is fn:parse-xml's own, not an internal one. More than one input
 : packet, so the parser had started before the conversion failed. :)
let $doc := concat('<?xml version="1.0" encoding="US-ASCII"?><a>',
                   string-join(for $i in 1 to 3000 return '<b/>', ''),
                   '<c>&#xE9;</c></a>')
return
  try { concat("PARSED ", count(fn:parse-xml($doc)//*)) }
  catch * { concat("DIAG ", $err:code, " ", $err:description) }
