jsoniq version "1.0";
(: A function item of a sequential function, never called here (no member is 0):
 : it could be called where the compiler does not see it, and the text is held in
 : memory. :)
import module namespace file = "http://expath.org/ns/file";
declare namespace an = "http://zorba.io/annotations";
declare variable $f external;
declare %an:sequential function local:touch() { file:append-text($f, ""); };
for $m in parse-json(file:read-text($f), { "zorba-streaming-members" : true })[]
return ($m, local:touch#0)[$m eq 0]
