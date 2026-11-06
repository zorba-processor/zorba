import module namespace file = "http://expath.org/ns/file";
declare namespace err = "http://www.w3.org/2005/xqt-errors";
declare variable $rbktPath as xs:string external;
declare variable $d := concat($rbktPath, "/Queries/zorba/bom/data/");
(: F&O: FOUT1190 if the resource "contains octets that cannot be decoded". :)
count(unparsed-text-lines(file:path-to-uri(concat($d, "utf16le-lone-surrogate.txt")), "UTF-16LE"))
