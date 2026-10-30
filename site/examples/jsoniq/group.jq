[
  for $o in parse-json(unparsed-text("../data/orders.json"))[]
  group by $c := $o.customer
  order by $c
  return {
    "customer": $c,
    "orders": count($o),
    "skus": [ distinct-values($o.items[].sku) ]
  }
]
