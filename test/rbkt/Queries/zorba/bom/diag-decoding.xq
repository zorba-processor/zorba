import module namespace file = "http://expath.org/ns/file";
declare namespace err = "http://www.w3.org/2005/xqt-errors";
declare variable $rbktPath as xs:string external;
declare variable $d := concat($rbktPath, "/Queries/zorba/bom/data/");
declare function local:show($s as xs:string*) as xs:string { string-join($s, "|") };
(: Temporary: what each function does with octets that cannot be decoded. :)
let $l := concat($d, "utf16le-lone-surrogate.txt")
let $t := concat($d, "utf16le-truncated.txt")
let $e := "UTF-16LE"
return (
  try { "ut-lone:", local:show(unparsed-text(file:path-to-uri($l), $e)) } catch * { "ut-lone-err:", local-name-from-QName($err:code) },
  try { "rt-lone:", local:show(file:read-text($l, $e)) } catch * { "rt-lone-err:", local-name-from-QName($err:code) },
  try { "ut-trunc:", local:show(unparsed-text(file:path-to-uri($t), $e)) } catch * { "ut-trunc-err:", local-name-from-QName($err:code) },
  try { "rt-trunc:", local:show(file:read-text($t, $e)) } catch * { "rt-trunc-err:", local-name-from-QName($err:code) },
  try { "utl-trunc:", local:show(unparsed-text-lines(file:path-to-uri($t), $e)) } catch * { "utl-trunc-err:", local-name-from-QName($err:code) },
  try { "rtl-trunc:", local:show(file:read-text-lines($t, $e)) } catch * { "rtl-trunc-err:", local-name-from-QName($err:code) }
)
