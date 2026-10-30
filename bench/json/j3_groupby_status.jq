(: j3: group by status :)
import module namespace file = "http://expath.org/ns/file";

declare variable $file as xs:string external;
declare variable $data := jn:parse-json(file:read-text($file));

for $o in $data.orders[]
group by $s := $o.status
order by $s
return { "status": $s, "orders": count($o), "avg_total": avg($o.total) }
