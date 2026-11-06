jsoniq version "1.0";
import module namespace file = "http://expath.org/ns/file";
declare variable $rbktPath as string external;
declare variable $d := $rbktPath || "/Queries/zorba/jsoniq-array-members/data/";
(: The option form keeps its behavior. :)
parse-json(file:read-text($d || "two.json"), { "zorba-streaming-members" : true, "jsoniq-multiple-top-level-items" : false })[]
