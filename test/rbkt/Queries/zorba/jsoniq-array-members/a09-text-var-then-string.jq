jsoniq version "1.0";
import module namespace file = "http://expath.org/ns/file";
declare variable $rbktPath as string external;
declare variable $d := $rbktPath || "/Queries/zorba/jsoniq-array-members/data/";
(: One text in a variable, used after a member was taken. :)
let $t := file:read-text($d || "ints.json") return [ parse-json($t)[][1], string-length($t) ]
