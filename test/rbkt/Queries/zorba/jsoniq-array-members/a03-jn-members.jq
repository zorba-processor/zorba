jsoniq version "1.0";
import module namespace file = "http://expath.org/ns/file";
declare variable $rbktPath as string external;
declare variable $d := $rbktPath || "/Queries/zorba/jsoniq-array-members/data/";
(: The function form of the unboxing. :)
[ members(parse-json(file:read-text($d || "ints.json"))) ]
