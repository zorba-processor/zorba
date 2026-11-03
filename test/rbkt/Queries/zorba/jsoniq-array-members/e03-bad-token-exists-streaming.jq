jsoniq version "1.0";
import module namespace file = "http://expath.org/ns/file";
declare variable $rbktPath as string external;
declare variable $d := $rbktPath || "/Queries/zorba/jsoniq-array-members/data/";
(: exists stops at the first member. :)
exists(parse-json(file:read-text($d || "bad-token.json"), { "zorba-streaming-members" : true })[])
