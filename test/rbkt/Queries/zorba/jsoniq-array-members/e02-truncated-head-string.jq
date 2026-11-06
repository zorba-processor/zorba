jsoniq version "1.0";
import module namespace file = "http://expath.org/ns/file";
declare variable $rbktPath as string external;
declare variable $d := $rbktPath || "/Queries/zorba/jsoniq-array-members/data/";
(: The same with head, from a string. :)
head(parse-json(concat(file:read-text($d || "bad-tail.json"), " "))[])
