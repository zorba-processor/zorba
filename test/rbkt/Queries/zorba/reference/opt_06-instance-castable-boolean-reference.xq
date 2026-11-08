(: instance of, castable as and fn:boolean evaluate their operand although its type decides the result. :)
import module namespace ref = "http://zorba.io/modules/reference";
let $a := <a/>, $b := <b/>, $c := <c/>
return (
  ref:reference($a) instance of xs:anyAtomicType, ref:has-reference($a),
  ref:reference($b) castable as xs:string, ref:has-reference($b),
  boolean(if (ref:reference($c)) then $c else $c), ref:has-reference($c)
)
