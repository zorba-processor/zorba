import module namespace dir = "http://zorba.io/modules/directory";

for $o in dir:documents($dir:root, "xml")("content")//order
let $customer := string($o/@customer)
group by $customer
order by $customer
return {
  "customer" : $customer,
  "orders" : count($o),
  "total" : sum(for $t in $o/total return xs:integer($t))
}
