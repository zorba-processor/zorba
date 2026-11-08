(: A let clause evaluates its domain although its variable is not used. :)
import module namespace ref = "http://zorba.io/modules/reference";
let $n := <a/>
return (let $x := ref:reference($n) return ref:has-reference($n))
