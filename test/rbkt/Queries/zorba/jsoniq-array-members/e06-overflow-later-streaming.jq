jsoniq version "1.0";
import module namespace file = "http://expath.org/ns/file";
declare variable $rbktPath as string external;
declare variable $d := $rbktPath || "/Queries/zorba/jsoniq-array-members/data/";
(: 1e999 after the member the query takes: xs:double rounds it to INF, which is not an error. :)
parse-json(file:read-text($d || "overflow.json"), { "zorba-streaming-members" : true })[][1]
