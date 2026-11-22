jsoniq version "1.0";
(: The parse alone: the number of events in the log. :)
import module namespace file = "http://expath.org/ns/file";
declare variable $log as string external;

count(parse-json(file:read-text($log), { "jsoniq-multiple-top-level-items" : true }))
