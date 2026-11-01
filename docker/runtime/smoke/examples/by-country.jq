import module namespace dir = "http://zorba.io/modules/directory";

let $orders := dir:documents($dir:root, "xml")("content")//order
for $c in dir:open($dir:root, "customers.csv")("content")()
return {
  "customer" : $c("name"),
  "country" : $c("country"),
  "total" : sum(for $t in $orders[@customer = $c("id")]/total
                return xs:integer($t))
}
