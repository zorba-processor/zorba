(: j5: sort all orders by total, top 10 :)
import module namespace file = "http://expath.org/ns/file";

declare variable $file as xs:string external;
declare variable $data := jn:parse-json(file:read-text($file));

(for $o in $data.orders[]
 order by $o.total descending, $o.id
 return $o.id)[position() le 10]
