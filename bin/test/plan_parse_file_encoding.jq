jsoniq version "1.0";
(: The same as plan_parse_file.jq, with the encoding written. :)
import module namespace file = "http://expath.org/ns/file";
declare variable $f external;
parse-json(file:read-text($f, "UTF-8"), { "zorba-streaming-members" : true })[]
