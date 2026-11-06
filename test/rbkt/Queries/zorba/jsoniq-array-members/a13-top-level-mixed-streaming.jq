jsoniq version "1.0";
import module namespace file = "http://expath.org/ns/file";
declare variable $rbktPath as string external;
declare variable $d := $rbktPath || "/Queries/zorba/jsoniq-array-members/data/";
(: Objects among top-level values have no members. :)
[ parse-json(file:read-text($d || "mixed.json"), { "zorba-streaming-members" : true })[] ]
