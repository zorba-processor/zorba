(: fn:exists calls ref:reference although its result type has exactly one item. :)
import module namespace ref = "http://zorba.io/modules/reference";
let $n := <a/> return (exists(ref:reference($n)), ref:has-reference($n))
