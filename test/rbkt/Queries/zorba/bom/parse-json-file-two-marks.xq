import module namespace file = "http://expath.org/ns/file";
declare variable $rbktPath as xs:string external;
declare variable $d := concat($rbktPath, "/Queries/zorba/bom/data/");
(: file:read-text takes the mark of the file; the string then starts with U+FEFF,
 : which jn:parse-json ignores, as for such a string in memory. :)
jn:members(jn:parse-json(file:read-text(concat($d, "two-marks.json"))))
