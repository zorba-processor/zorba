jsoniq version "1.0";
import module namespace file = "http://expath.org/ns/file";
declare variable $rbktPath as string external;
declare variable $d := $rbktPath || "/Queries/zorba/jsoniq-array-members/data/";
(: Three marks: file:read-text takes the first, parse-json ignores the second, and
 : the third is not JSON. :)
[ parse-json(file:read-text($d || "three-marks.json"), { "zorba-streaming-members" : true })[] ]
