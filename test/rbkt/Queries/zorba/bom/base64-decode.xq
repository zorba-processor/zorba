import module namespace file = "http://expath.org/ns/file";
import module namespace base64 = "http://zorba.io/modules/base64";
declare variable $rbktPath as xs:string external;
declare variable $d := concat($rbktPath, "/Queries/zorba/bom/data/");
(: Decoding binary data is not reading a text resource: the mark stays. :)
string-to-codepoints(base64:decode(
  file:read-binary(concat($d, "bom.txt"))))[1]
