for $o in parse-json(unparsed-text("../data/orders.json"))[]
return {
  "id": $o.id,
  "coupon is null": $o.coupon eq null,
  "coupon missing": empty($o.coupon),
  "coupon or default": if (empty($o.coupon) or $o.coupon eq null)
                       then "none" else $o.coupon
}
