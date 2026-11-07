declare namespace err = "http://www.w3.org/2005/xqt-errors";
(: The declared encoding does not cover a character of the fragment: the
 : parser stops part way through, after some elements were started. The
 : error is fn:parse-xml-fragment's own, not an internal one. :)
let $frag := concat('<?xml encoding="US-ASCII"?><a>',
                    string-join(for $i in 1 to 3000 return '<b/>', ''),
                    '<c>&#xE9;</c></a>')
return
  try { fn:parse-xml-fragment($frag) }
  catch err:FODC0006 { not(contains($err:description, "assertion failed")) }
