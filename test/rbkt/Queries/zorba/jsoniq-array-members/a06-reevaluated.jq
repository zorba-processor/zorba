jsoniq version "1.0";
import module namespace file = "http://expath.org/ns/file";
declare variable $rbktPath as string external;
declare variable $d := $rbktPath || "/Queries/zorba/jsoniq-array-members/data/";
(: Every evaluation parses again. :)
[ for $i in 1 to 3 return parse-json(file:read-text($d || "ints.json"))[][$i] ]
