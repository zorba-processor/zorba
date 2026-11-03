import module namespace file = "http://expath.org/ns/file";
import module namespace fetch = "http://zorba.io/modules/fetch";
declare variable $rbktPath as xs:string external;
declare variable $d := concat($rbktPath, "/Queries/zorba/bom/data/");
string-to-codepoints(fetch:content(concat("file://", $d, "bom.txt")))[1]
