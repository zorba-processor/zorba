import module namespace dir = "http://zorba.io/modules/directory";

let $paid :=
  for $p in dir:open($dir:root, "payments/payments.json")("content")()
  where $p("status") eq "paid"
  return $p("order")
for $o in dir:documents($dir:root, "xml")("content")//order
where not(string($o/@id) = $paid)
return {
  "order" : string($o/@id),
  "customer" : string($o/@customer),
  "total" : xs:integer($o/total)
}
