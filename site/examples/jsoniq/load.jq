let $orders := parse-json(unparsed-text("../data/orders.json"))[]
return [ count($orders), $orders.customer ]
