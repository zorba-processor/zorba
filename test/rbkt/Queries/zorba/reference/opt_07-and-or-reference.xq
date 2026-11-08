(: An "and" or "or" with a constant operand evaluates its other operand first, as at O0. :)
import module namespace ref = "http://zorba.io/modules/reference";
let $a := <a/>, $b := <b/>
return (
  (string-length(ref:reference($a)) gt 0 and false()), ref:has-reference($a),
  (string-length(ref:reference($b)) gt 0 or true()), ref:has-reference($b)
)
