(: Validation: 2,000 constructed orders validated against orders.xsd, then a
 : sum over their typed values. Every run validates through Xerces. :)
import schema namespace o = "urn:zorba:bench:orders" at "orders.xsd";

let $orders := validate { <o:orders>{
  for $i in 1 to 2000
  return <o:order id="{$i}"><o:qty>{$i mod 7}</o:qty><o:price>{$i div 4}</o:price></o:order>
}</o:orders> }
return sum($orders/o:order[o:qty gt 3]/(o:qty * o:price))
