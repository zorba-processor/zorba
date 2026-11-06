jsoniq version "1.0";
(: A call of a function that takes a function, with a function item that cannot
 : write: the rule does not look into it, and the text is held in memory. :)
import module namespace file = "http://expath.org/ns/file";
declare variable $f external;
for $m in parse-json(file:read-text($f), { "zorba-streaming-members" : true })[]
return for-each($m, function($x) { $x + 1 })
