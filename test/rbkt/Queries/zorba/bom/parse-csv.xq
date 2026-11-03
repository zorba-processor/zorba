import module namespace file = "http://expath.org/ns/file";
import module namespace csv = "http://zorba.io/modules/json-csv";
declare variable $rbktPath as xs:string external;
declare variable $d := concat($rbktPath, "/Queries/zorba/bom/data/");
string-join(
  jn:keys(csv:parse(file:read-text(concat($d, "bom.csv")))),
  ",")
