(: j4: join orders with customers, group by country :)
import module namespace file = "http://expath.org/ns/file";

declare variable $file as xs:string external;
declare variable $data := jn:parse-json(file:read-text($file));

for $o in $data.orders[]
for $c in $data.customers[]
where $o.customer eq $c.id
group by $country := $c.country
order by $country
return { "country": $country, "orders": count($o), "revenue": sum($o.total) }
