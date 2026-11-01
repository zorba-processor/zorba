(: Opening one document per format: XML as a document node, JSON as the
 : parsed item, CSV as an array of row objects; the path is normalized. :)
import module namespace dir = "http://zorba.io/modules/directory";
import module namespace jn = "http://jsoniq.org/functions";

declare variable $rbktPath as xs:string external;
declare variable $root := concat($rbktPath, "/Queries/zorba/directory/tree");

let $x := dir:open($root, "orders/2026/q1.xml")("content")
let $j := dir:open($root, "payments/late.json")("content")
let $c := dir:open($root, "customers.csv")("content")
let $p := dir:open($root, "./payments//payments.json")
return string-join((
  string($x instance of document-node()),
  string-join($x/orders/order/@id, ","),
  $j("status"),
  string-join(for $r in jn:members($c) return $r("name"), ","),
  $p("path"),
  string(jn:size($p("content")))
), " ")
