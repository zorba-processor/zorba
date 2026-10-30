(: j7: group by customer, ten biggest spenders :)
import module namespace file = "http://expath.org/ns/file";

declare variable $file as xs:string external;
declare variable $data := jn:parse-json(file:read-text($file));

(for $o in $data.orders[]
 group by $c := $o.customer
 let $spent := sum($o.total)
 order by $spent descending, $c
 return { "customer": $c, "orders": count($o), "spent": $spent })[position() le 10]
