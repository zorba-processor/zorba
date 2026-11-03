import module namespace file = "http://expath.org/ns/file";
declare variable $rbktPath as xs:string external;
declare variable $d := concat($rbktPath, "/Queries/zorba/bom/data/");
(: The stream is read twice: the second read also starts after the mark. :)
let $s := file:read-text(concat($d, "bom.json"))
return (jn:members(jn:parse-json($s)), jn:members(jn:parse-json($s)))
