import module namespace file = "http://expath.org/ns/file";
declare namespace err = "http://www.w3.org/2005/xqt-errors";
declare variable $rbktPath as xs:string external;
declare variable $d := concat($rbktPath, "/Queries/zorba/bom/data/");
(: file:read-text-lines declares FOFL9999 for any other error. :)
count(file:read-text-lines(concat($d, "utf16le-lone-surrogate.txt"), "UTF-16LE"))
