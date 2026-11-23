jsoniq version "1.0";
(: unmatched.jq with the distinct user ids of the successful requests in a
 : global variable instead of a let clause. :)
import module namespace file = "http://expath.org/ns/file";
declare namespace an = "http://zorba.io/annotations";
declare variable $log as string external;

(: Each call parses the log again, streaming. :)
declare %an:nondeterministic function local:events()
{
  parse-json(file:read-text($log), { "jsoniq-multiple-top-level-items" : true })
};

declare variable $succeeded := distinct-values(local:events()[$$.http.status eq 200].user.id);

for $e in local:events()
where $e.http.status ge 500 and not($e.user.id = $succeeded)
group by $service := $e.service
order by $service
return { "service" : $service, "errors" : count($e) }
