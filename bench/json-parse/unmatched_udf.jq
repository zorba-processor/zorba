jsoniq version "1.0";
(: unmatched.jq with the comparison in a function that is not inlined (it
 : recurses), called for each event with the distinct user ids as argument. :)
import module namespace file = "http://expath.org/ns/file";
declare namespace an = "http://zorba.io/annotations";
declare variable $log as string external;

(: Each call parses the log again, streaming. :)
declare %an:nondeterministic function local:events()
{
  parse-json(file:read-text($log), { "jsoniq-multiple-top-level-items" : true })
};

(: Whether $id is among $ids; the recursion keeps the function from being inlined. :)
declare function local:known($id, $ids, $depth)
{
  if ($depth gt 0) then local:known($id, $ids, $depth - 1) else $id = $ids
};

let $succeeded := distinct-values(local:events()[$$.http.status eq 200].user.id)
for $e in local:events()
where $e.http.status ge 500 and not(local:known($e.user.id, $succeeded, 0))
group by $service := $e.service
order by $service
return { "service" : $service, "errors" : count($e) }
