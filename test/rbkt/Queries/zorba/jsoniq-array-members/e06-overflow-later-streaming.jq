jsoniq version "1.0";
import module namespace file = "http://expath.org/ns/file";
declare variable $rbktPath as string external;
declare variable $d := $rbktPath || "/Queries/zorba/jsoniq-array-members/data/";
(: A number out of the range of xs:double after the member the query takes: not an error. :)
parse-json(file:read-text($d || "overflow.json"), { "zorba-streaming-members" : true })[][1]
