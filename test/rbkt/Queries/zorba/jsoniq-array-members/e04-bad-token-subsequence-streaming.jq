jsoniq version "1.0";
import module namespace file = "http://expath.org/ns/file";
declare variable $rbktPath as string external;
declare variable $d := $rbktPath || "/Queries/zorba/jsoniq-array-members/data/";
(: subsequence stops after its range. :)
subsequence(parse-json(concat(file:read-text($d || "bad-token.json"), " "), { "zorba-streaming-members" : true })[], 1, 1)
