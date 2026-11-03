import module namespace file = "http://expath.org/ns/file";
declare variable $rbktPath as xs:string external;
declare variable $d := concat($rbktPath, "/Queries/zorba/bom/data/");
(: A file shorter than a mark still reads whole. :)
let $f := concat($d, "short.txt")
return (string(file:read-text($f)), file:read-text-lines($f))
