import module namespace file = "http://expath.org/ns/file";
declare variable $rbktPath as xs:string external;
declare variable $d := concat($rbktPath, "/Queries/zorba/bom/data/");
parse-xml(file:read-text(concat($d, "bom.xml")))/a/string()
