import module namespace file = "http://expath.org/ns/file";
declare variable $rbktPath as xs:string external;
declare variable $d := concat($rbktPath, "/Queries/zorba/bom/data/");
let $lines := file:read-text-lines(concat($d, "bom.txt"))
return (count($lines), string-to-codepoints($lines[1])[1])
