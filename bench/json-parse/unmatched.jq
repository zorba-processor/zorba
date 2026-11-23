jsoniq version "1.0";
(: Per service, the server errors of users who have no successful request in
 : the log: a general = against the distinct user ids of a second parse. :)
import module namespace file = "http://expath.org/ns/file";
declare namespace an = "http://zorba.io/annotations";
declare variable $log as string external;

(: Each call parses the log again, streaming. :)
declare %an:nondeterministic function local:events()
{
  parse-json(file:read-text($log), { "jsoniq-multiple-top-level-items" : true })
};

let $succeeded := distinct-values(local:events()[$$.http.status eq 200].user.id)
for $e in local:events()
where $e.http.status ge 500 and not($e.user.id = $succeeded)
group by $service := $e.service
order by $service
return { "service" : $service, "errors" : count($e) }
