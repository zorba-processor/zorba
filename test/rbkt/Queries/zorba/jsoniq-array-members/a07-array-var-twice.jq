jsoniq version "1.0";
import module namespace file = "http://expath.org/ns/file";
declare variable $rbktPath as string external;
declare variable $d := $rbktPath || "/Queries/zorba/jsoniq-array-members/data/";
(: An array in a variable: size and two unboxings. :)
let $a := parse-json(file:read-text($d || "ints.json")) return [ size($a), count($a[]), sum($a[]), [ $a[] ] ]
