jsoniq version "1.0";
(: The server errors of one service, projected to a few fields. :)
import module namespace file = "http://expath.org/ns/file";
declare variable $log as string external;

for $e in parse-json(file:read-text($log), { "jsoniq-multiple-top-level-items" : true })
where $e.service eq "checkout" and $e.http.status ge 500
return { "ts" : $e.ts, "trace" : $e.trace.id, "path" : $e.http.path, "status" : $e.http.status }
