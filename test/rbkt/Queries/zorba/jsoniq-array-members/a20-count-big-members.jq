jsoniq version "1.0";
import module namespace file = "http://expath.org/ns/file";
declare variable $rbktPath as string external;
declare variable $d := $rbktPath || "/Queries/zorba/jsoniq-array-members/data/";
(: Taking the first member of many. :)
parse-json(concat("[", string-join(for $i in 1 to 1000 return string($i), ","), "]"))[][1000]
