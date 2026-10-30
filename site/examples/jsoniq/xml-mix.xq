(: XQuery syntax with JSON values: call an object with a key, an array with
   a position, and $a() returns all members. :)
import module namespace jn = "http://jsoniq.org/functions";

let $orders := jn:parse-json(unparsed-text("../data/orders.json"))
return
  <orders>{
    for $o in $orders()
    return <order id="{$o("id")}" customer="{$o("customer")}">{
      for $i in $o("items")()
      return <item sku="{$i("sku")}" qty="{$i("qty")}"/>
    }</order>
  }</orders>
