(: j6: distinct SKUs across all line items :)
import module namespace file = "http://expath.org/ns/file";

declare variable $file as xs:string external;
declare variable $data := jn:parse-json(file:read-text($file));

count(distinct-values($data.orders[].items[].sku))
