jsoniq version "1.0";
(: The dynamic call is in the body of a function that is not inlined (it is
 : recursive): the rule follows calls into bodies, and the text is held in
 : memory. The function item itself cannot write. :)
import module namespace file = "http://expath.org/ns/file";
declare variable $f external;
declare function local:apply($g, $x, $n as integer)
{ if ($n gt 0) then local:apply($g, $x, $n - 1) else $g($x) };
for $m in parse-json(file:read-text($f), { "zorba-streaming-members" : true })[]
return local:apply(function($y) { $y }, $m, 1)
