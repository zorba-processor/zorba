(: A for clause evaluates its domain although its variable is not used. :)
import module namespace ref = "http://zorba.io/modules/reference";
let $n := <a/>
return (for $r in ref:reference($n) return ref:has-reference($n))
