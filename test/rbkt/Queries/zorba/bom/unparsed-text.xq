import module namespace file = "http://expath.org/ns/file";
declare variable $rbktPath as xs:string external;
declare variable $d := concat($rbktPath, "/Queries/zorba/bom/data/");
let $u := file:path-to-uri(concat($d, "bom.txt"))
let $lines := unparsed-text-lines($u)
return (
  string-to-codepoints(unparsed-text($u))[1],
  count($lines),
  string-to-codepoints($lines[1])[1]
)
