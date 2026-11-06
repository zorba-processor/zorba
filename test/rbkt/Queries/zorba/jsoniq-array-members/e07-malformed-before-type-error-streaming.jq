jsoniq version "1.0";
import module namespace file = "http://expath.org/ns/file";
declare variable $rbktPath as string external;
declare variable $d := $rbktPath || "/Queries/zorba/jsoniq-array-members/data/";
(: A member would raise a type error, but the array is malformed after it. :)
for $m in parse-json(file:read-text($d || "type-then-bad.json"), { "zorba-streaming-members" : true })[] return $m.v + 1
