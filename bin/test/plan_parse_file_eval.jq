jsoniq version "1.0";
(: Eval runs code the compiler does not see: the text is held in memory. :)
import module namespace file = "http://expath.org/ns/file";
import module namespace reflection = "http://zorba.io/modules/reflection";
declare variable $f external;
for $m in parse-json(file:read-text($f), { "zorba-streaming-members" : true })[]
return reflection:eval(string($m))
