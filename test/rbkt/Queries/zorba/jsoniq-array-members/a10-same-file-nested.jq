jsoniq version "1.0";
import module namespace file = "http://expath.org/ns/file";
declare variable $rbktPath as string external;
declare variable $d := $rbktPath || "/Queries/zorba/jsoniq-array-members/data/";
(: The same file read by two nested iterations. :)
[ for $a in parse-json(file:read-text($d || "ints.json"))[] for $b in parse-json(file:read-text($d || "ints.json"))[] return $a * 10 + $b ]
