import module namespace file = "http://expath.org/ns/file";
declare variable $rbktPath as xs:string external;
declare variable $d := concat($rbktPath, "/Queries/zorba/bom/data/");
(: A byte order mark is not part of the string: neither when the string is
 : materialized nor when it is serialized from its stream. :)
let $f := concat($d, "bom.txt")
return (
  string-to-codepoints(file:read-text($f))[1],
  string-length(file:read-text($f)),
  string-to-codepoints(string(file:read-text($f)))[1],
  string-to-codepoints(serialize(file:read-text($f)))[1]
)
