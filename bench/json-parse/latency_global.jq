jsoniq version "1.0";
(: latency.jq with the events in a global variable (declare variable). :)
import module namespace file = "http://expath.org/ns/file";
declare variable $log as string external;

declare variable $events := parse-json(file:read-text($log), { "jsoniq-multiple-top-level-items" : true });

for $e in $events
group by $service := $e.service, $route := $e.http.route
let $ms := $e.http.duration_ms
order by avg($ms) descending, $service, $route
return {
  "service" : $service,
  "route" : $route,
  "requests" : count($e),
  "avg_ms" : round(avg($ms), 3),
  "max_ms" : max($ms),
  "server_errors" : count($e[$$.http.status ge 500])
}
