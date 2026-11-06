jsoniq version "1.0";
(: An update, of a copy here: the rule counts any update, and the text is held in
 : memory. :)
import module namespace file = "http://expath.org/ns/file";
declare variable $f external;
declare updating function local:mark($o) { insert json { "seen" : true } into $o };
for $m in parse-json(file:read-text($f), { "zorba-streaming-members" : true })[]
return copy $c := { "m" : $m } modify local:mark($c) return $c
