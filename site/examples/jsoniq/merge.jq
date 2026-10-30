let $defaults := { "color": "blue", "size": "M" }
let $override := { "size": "L", "qty": 3 }
return [
  {| $override, { "color": $defaults.color } |},
  {| for $k in ("x", "y") return { $k: upper-case($k) } |}
]
