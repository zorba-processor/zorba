let $order := {
  "id": 1,
  "customer": { "name": "ana", "city": "Monterrey" },
  "items": [ { "sku": "A1", "qty": 2 }, { "sku": "B7", "qty": 1 } ],
  "ship to": "store"
}
return [
  $order.customer.name,
  $order.items[[2]].sku,
  $order.items[].sku,
  $order."ship to",
  let $key := "id" return $order.$key,
  count($order.items[])
]
