import module namespace file = "http://expath.org/ns/file";
declare variable $rbktPath as xs:string external;
declare variable $d := concat($rbktPath, "/Queries/zorba/bom/data/");
(: F&O: lines end with x0A, x0D x0A or x0D, and a last empty line does not count. :)
let $f := concat($d, "lines.txt")
let $g := concat($d, "lines-blank.txt")
return (
  string-join(file:read-text-lines($f), "|"),
  string-join(unparsed-text-lines(file:path-to-uri($f)), "|"),
  count(file:read-text-lines($g)),
  count(unparsed-text-lines(file:path-to-uri($g)))
)
