jsoniq version "1.0";
import module namespace file = "http://expath.org/ns/file";
declare variable $rbktPath as string external;
declare variable $d := $rbktPath || "/Queries/zorba/jsoniq-array-members/data/";
(: A filter, an aggregate and a window over the members. :)
[ [ parse-json(file:read-text($d || "ints.json"), { "zorba-streaming-members" : true })[][$$ gt 1] ], sum(parse-json(file:read-text($d || "ints.json"), { "zorba-streaming-members" : true })[]), [ for tumbling window $w in parse-json(file:read-text($d || "ints.json"), { "zorba-streaming-members" : true })[] start at $s when true end at $e when $e - $s eq 1 return [ $w ] ] ]
