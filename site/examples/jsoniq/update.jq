let $order := { "id": 2, "customer": "luis", "status": "pending",
                "coupon": null, "items": [ "A1" ] }
return
  copy $o := $order
  modify (
    replace value of json $o.status with "shipped",
    delete json $o.coupon,
    rename json $o.customer as "client",
    insert json { "carrier": "dhl" } into $o,
    append json "B7" into $o.items
  )
  return $o
