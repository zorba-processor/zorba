jsoniq version "1.0";
import module namespace file = "http://expath.org/ns/file";
declare variable $rbktPath as string external;
declare variable $d := $rbktPath || "/Queries/zorba/jsoniq-array-members/data/";
(: The same from a string. :)
parse-json(concat(file:read-text($d || "then-bad.json"), " "))[][1]
