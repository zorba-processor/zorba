[
  for $o in parse-json(unparsed-text("../data/orders.json"))[]
  let $total := sum(for $i in $o.items[] return $i.qty * $i.price)
  where $o.status eq "shipped"
  order by $total descending
  return { "order": $o.id, "customer": $o.customer, "total": $total }
]
