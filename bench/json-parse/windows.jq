jsoniq version "1.0";
(: Requests, server errors and the slowest request per minute, with a tumbling window
 : over the log. :)
import module namespace file = "http://expath.org/ns/file";
declare variable $log as string external;

for tumbling window $w in parse-json(file:read-text($log), { "jsoniq-multiple-top-level-items" : true })
  start $first when true()
  end next $n when substring($n.ts, 1, 16) ne substring($first.ts, 1, 16)
return {
  "minute" : substring($first.ts, 1, 16),
  "requests" : count($w),
  "server_errors" : count($w[$$.http.status ge 500]),
  "max_ms" : max($w.http.duration_ms)
}
