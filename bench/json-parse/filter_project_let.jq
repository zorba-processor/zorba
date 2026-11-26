jsoniq version "1.0";
(: filter_project.jq with the events bound by a let clause. :)
import module namespace file = "http://expath.org/ns/file";
declare variable $log as string external;

let $events := parse-json(file:read-text($log), { "jsoniq-multiple-top-level-items" : true })
for $e in $events
where $e.service eq "checkout" and $e.http.status ge 500
return { "ts" : $e.ts, "trace" : $e.trace.id, "path" : $e.http.path, "status" : $e.http.status }
