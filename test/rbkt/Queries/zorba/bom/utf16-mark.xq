import module namespace file = "http://expath.org/ns/file";
declare variable $rbktPath as xs:string external;
declare variable $d := concat($rbktPath, "/Queries/zorba/bom/data/");
(: In another encoding the mark decodes to U+FEFF, which is not kept either. :)
string-to-codepoints(file:read-text(concat($d, "utf16le.txt"), "UTF-16LE"))
