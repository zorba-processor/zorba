(: A comparison of fn:count with 0 or 1 evaluates every item of the argument. :)
import module namespace ref = "http://zorba.io/modules/reference";
let $a := <a/>, $b := <b/>, $c := <c/>, $d := <d/>, $e := <e/>
return (
  count((ref:reference($a), ref:reference($b))) = 0, ref:has-reference($b),
  count((ref:reference($c), ref:reference($d), ref:reference($e))) = 1, ref:has-reference($e)
)
