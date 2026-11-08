(: fn:count of a FLWOR with one let clause evaluates the domain of the let. :)
import module namespace ref = "http://zorba.io/modules/reference";
let $n := <a/>
return (count(let $x := ref:reference($n) return 1), ref:has-reference($n))
