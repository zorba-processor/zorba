jsoniq version "1.0";
(: Each event as one flat object (JSON lines). :)
import module namespace file = "http://expath.org/ns/file";
declare variable $log as string external;

for $e in parse-json(file:read-text($log), { "jsoniq-multiple-top-level-items" : true })
return {
  "ts" : $e.ts,
  "level" : $e.level,
  "service" : $e.service,
  "trace" : $e.trace.id,
  "method" : $e.http.method,
  "status" : $e.http.status,
  "duration_ms" : $e.http.duration_ms,
  "country" : $e.client.geo.country,
  "user" : $e.user.id,
  "team" : $e.user.team
}
