jsoniq version "1.0";
(: No sequential expression, no function item, no dynamic call: the file stays in
 : the file. :)
import module namespace file = "http://expath.org/ns/file";
declare variable $f external;
parse-json(file:read-text($f), { "zorba-streaming-members" : true })[]
