jsoniq version "1.0";
(: latency.jq with the request of each event bound by a let clause. :)
import module namespace file = "http://expath.org/ns/file";
declare variable $log as string external;

for $e in parse-json(file:read-text($log), { "jsoniq-multiple-top-level-items" : true })
let $http := $e.http
group by $service := $e.service, $route := $http.route
let $ms := $http.duration_ms
order by avg($ms) descending, $service, $route
return {
  "service" : $service,
  "route" : $route,
  "requests" : count($e),
  "avg_ms" : round(avg($ms), 3),
  "max_ms" : max($ms),
  "server_errors" : count($http[$$.status ge 500])
}
