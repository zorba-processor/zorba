(: The same navigation in XQuery syntax (.xq): no dot lookup, no null
   literal, no $$. Objects and arrays are called like functions. :)
import module namespace jn = "http://jsoniq.org/functions";

let $o := { "name": "ana", "tags": [ "a", "b", "c" ], "coupon": jn:null() }
return [
  $o("name"),
  $o("tags")(2),
  $o("tags")(),
  jn:keys($o),
  jn:size($o("tags")),
  $o("coupon") instance of js:null,
  jn:members($o("tags")) ! upper-case(.)
]
