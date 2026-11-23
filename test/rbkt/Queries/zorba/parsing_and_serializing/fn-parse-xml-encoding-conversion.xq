(: The input is a string, already decoded, so its encoding declaration has no
 : meaning (F&O 3.1, 14.7.1, Notes): the document parses although UTF-7 has
 : no direct form for '~'. The input is more than one packet of the loader
 : (8192 bytes). doc-encoding-conversion tests the conversion failure. :)
let $doc := concat('<?xml version="1.0" encoding="UTF-7"?><a>',
                   string-join(for $i in 1 to 3000 return '<b/>', ''),
                   '<c>~</c></a>')
return count(fn:parse-xml($doc)/a/*) eq 3001
