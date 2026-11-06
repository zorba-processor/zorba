jsoniq version "1.0";
import module namespace file = "http://expath.org/ns/file";
declare variable $rbktPath as string external;
declare variable $d := $rbktPath || "/Queries/zorba/jsoniq-array-members/data/";
(: All kinds of member, from a string. :)
[ parse-json(concat(file:read-text($d || "members.json"), " "), { "zorba-streaming-members" : true })[] ]
