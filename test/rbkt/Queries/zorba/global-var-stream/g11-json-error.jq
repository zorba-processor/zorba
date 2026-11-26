jsoniq version "1.0";
(: A parse error in the global is raised at every level. :)
declare variable $events := parse-json("{ \"a\" : 1 }
{ \"a\" : ", { "jsoniq-multiple-top-level-items" : true });

for $e in $events
return $e.a
