import module namespace file = "http://expath.org/ns/file";
declare variable $rbktPath as xs:string external;
declare variable $d := concat($rbktPath, "/Queries/zorba/bom/data/");
(: Unicode D98: in the UTF-16 scheme the initial mark says the byte order and is
 : not text. The U+FEFF after it is text. :)
let $f := concat($d, "utf16-sig-mark.txt")
return (string-to-codepoints(file:read-text($f, "UTF-16")),
        string-to-codepoints(unparsed-text(file:path-to-uri($f), "UTF-16")))
