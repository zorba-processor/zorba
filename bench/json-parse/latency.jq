jsoniq version "1.0";
(: Request latency by service and route, slowest first. :)
import module namespace file = "http://expath.org/ns/file";
declare variable $log as string external;

for $e in parse-json(file:read-text($log), { "jsoniq-multiple-top-level-items" : true })
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
