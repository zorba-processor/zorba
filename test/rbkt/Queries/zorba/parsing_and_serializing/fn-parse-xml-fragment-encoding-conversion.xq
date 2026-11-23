(: The input is a string, already decoded, so the encoding of its text
 : declaration has no meaning (F&O 3.1, 14.7.2, which refers to the notes of
 : fn:parse-xml): the fragment parses although UTF-7 has no direct form for
 : '~'. :)
let $frag := concat('<?xml encoding="UTF-7"?><a>',
                    string-join(for $i in 1 to 3000 return '<b/>', ''),
                    '<c>~</c></a>')
return count(fn:parse-xml-fragment($frag)/a/*) eq 3001
