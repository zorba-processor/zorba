(: The same navigation in XQuery syntax (.xq): no dot lookup, no null
   literal, no $$. Objects and arrays are called like functions. :)
let $o := { "name": "ana", "tags": [ "a", "b", "c" ], "coupon": jn:null() }
return [
  $o("name"),
  $o("tags")(2),
  $o("tags")(),
  keys($o),
  size($o("tags")),
  $o("coupon") instance of js:null,
  members($o("tags")) ! upper-case(.)
]
