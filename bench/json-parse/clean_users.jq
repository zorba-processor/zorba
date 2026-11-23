jsoniq version "1.0";
(: Requests per service of the users who never got a server error: a FLWOR gives the
 : events of those users to an enclosing group by. :)
import module namespace file = "http://expath.org/ns/file";
declare variable $log as string external;

for $x in (
  for $e in parse-json(file:read-text($log), { "jsoniq-multiple-top-level-items" : true })
  group by $user := $e.user.id
  where every $status in $e.http.status satisfies $status lt 500
  return $e
)
group by $service := $x.service
order by $service
return { "service" : $service, "requests" : count($x) }
