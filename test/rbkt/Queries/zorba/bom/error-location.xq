import module namespace file = "http://expath.org/ns/file";
declare variable $rbktPath as xs:string external;
declare variable $d := concat($rbktPath, "/Queries/zorba/bom/data/");
(: The mark does not count for the column of an error. :)
try { jn:parse-json(file:read-text(concat($d, "bad.json"))) }
catch * { $err:line-number, $err:column-number }
