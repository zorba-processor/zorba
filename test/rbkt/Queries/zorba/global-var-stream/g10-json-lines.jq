jsoniq version "1.0";
(: Events of a JSON lines string in a global variable, grouped. :)
declare variable $lines := "{ \"service\" : \"a\", \"ms\" : 3 }
{ \"service\" : \"b\", \"ms\" : 5 }
{ \"service\" : \"a\", \"ms\" : 4 }";

declare variable $events := parse-json($lines, { "jsoniq-multiple-top-level-items" : true });

for $e in $events
group by $service := $e.service
order by $service
return { "service" : $service, "requests" : count($e), "ms" : sum($e.ms) }
