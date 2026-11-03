jsoniq version "1.0";
import module namespace file = "http://expath.org/ns/file";
declare variable $rbktPath as string external;
declare variable $d := $rbktPath || "/Queries/zorba/jsoniq-array-members/data/";
(: A number that does not fit after the member the query takes. :)
parse-json(file:read-text($d || "overflow.json"))[][1]
