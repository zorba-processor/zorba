import module namespace file = "http://expath.org/ns/file";
declare variable $rbktPath as xs:string external;
declare variable $d := concat($rbktPath, "/Queries/zorba/bom/data/");
(: Only a mark at the start of the file is one; the next U+FEFF is a character. :)
string-to-codepoints(file:read-text(concat($d, "two-marks.txt")))
