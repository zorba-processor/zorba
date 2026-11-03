import module namespace file = "http://expath.org/ns/file";
declare variable $rbktPath as xs:string external;
declare variable $d := concat($rbktPath, "/Queries/zorba/bom/data/");
(: Unicode D97: in UTF-16LE an initial <FF FE> is U+FEFF ZERO WIDTH NO-BREAK SPACE,
 : part of the text, rather than a byte order mark. :)
string-to-codepoints(file:read-text(concat($d, "utf16le.txt"), "UTF-16LE"))
