(: j1: navigation, sum of line item values :)
import module namespace file = "http://expath.org/ns/file";

declare variable $file as xs:string external;
declare variable $data := jn:parse-json(file:read-text($file));

sum(for $i in $data.orders[].items[] return $i.qty * $i.price)
