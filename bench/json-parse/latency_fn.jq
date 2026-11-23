jsoniq version "1.0";
(: latency.jq with the parse in a function, as queries that read the log more than
 : once write it, and the status of the slowest request of each group. :)
import module namespace file = "http://expath.org/ns/file";
declare namespace an = "http://zorba.io/annotations";
declare variable $log as string external;
declare %an:nondeterministic function local:events()
{ parse-json(file:read-text($log), { "jsoniq-multiple-top-level-items" : true }) };

for $e in local:events()
group by $service := $e.service, $route := $e.http.route
let $ms := $e.http.duration_ms
let $slowest := for $v in $e order by $v.http.duration_ms descending return $v
order by avg($ms) descending, $service, $route
return {
  "service" : $service,
  "route" : $route,
  "requests" : count($e),
  "avg_ms" : round(avg($ms), 3),
  "max_ms" : max($ms),
  "slowest_status" : $slowest[1].http.status,
  "server_errors" : count($e[$$.http.status ge 500])
}
