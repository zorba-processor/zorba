(: fn:empty, fn:count and a comparison of the count call ref:reference although its result type has exactly one item. :)
import module namespace ref = "http://zorba.io/modules/reference";
let $a := <a/>, $b := <b/>, $c := <c/>
return (
  empty(ref:reference($a)), ref:has-reference($a),
  count(ref:reference($b)), ref:has-reference($b),
  count(ref:reference($c)) = -1, ref:has-reference($c)
)
