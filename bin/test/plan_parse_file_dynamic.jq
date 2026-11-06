jsoniq version "1.0";
(: The main body is not sequential, but it calls a sequential function through a
 : function item (never, here: no member is 0). The text is held in memory. :)
import module namespace file = "http://expath.org/ns/file";
declare namespace an = "http://zorba.io/annotations";
declare variable $f external;
declare %an:sequential function local:touch() { file:append-text($f, ""); };
for $m in parse-json(file:read-text($f), { "zorba-streaming-members" : true })[]
return ($m, if ($m eq 0) then local:touch#0() else ())
