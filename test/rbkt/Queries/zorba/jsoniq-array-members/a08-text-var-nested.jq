jsoniq version "1.0";
import module namespace file = "http://expath.org/ns/file";
declare variable $rbktPath as string external;
declare variable $d := $rbktPath || "/Queries/zorba/jsoniq-array-members/data/";
(: One text in a variable, parsed by two nested iterations. :)
let $t := file:read-text($d || "ints.json") return [ for $a in parse-json($t)[] for $b in parse-json($t)[] return $a * 10 + $b ]
