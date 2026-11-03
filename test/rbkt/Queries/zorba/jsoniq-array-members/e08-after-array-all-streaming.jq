jsoniq version "1.0";
import module namespace file = "http://expath.org/ns/file";
declare variable $rbktPath as string external;
declare variable $d := $rbktPath || "/Queries/zorba/jsoniq-array-members/data/";
(: Malformed after a complete array, all members wanted. :)
[ parse-json(file:read-text($d || "then-bad.json"), { "zorba-streaming-members" : true })[] ]
