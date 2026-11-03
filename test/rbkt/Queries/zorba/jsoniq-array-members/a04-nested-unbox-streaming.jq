jsoniq version "1.0";
import module namespace file = "http://expath.org/ns/file";
declare variable $rbktPath as string external;
declare variable $d := $rbktPath || "/Queries/zorba/jsoniq-array-members/data/";
(: Unboxing the unboxed members. :)
[ parse-json(file:read-text($d || "nested.json"), { "zorba-streaming-members" : true })[][] ]
