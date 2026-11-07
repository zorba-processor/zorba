declare namespace err = "http://www.w3.org/2005/xqt-errors";
for $n in (10, 1000, 2040, 3000, 5000)
let $doc := concat('<?xml version="1.0" encoding="UTF-7"?><a>',
                   string-join(for $i in 1 to $n return '<b/>', ''),
                   '<c>~</c></a>')
return
  try { concat("DIAG ", $n, " PARSED ", count(fn:parse-xml($doc)//*)) }
  catch * { concat("DIAG ", $n, " ", $err:code, " ", $err:description) }
