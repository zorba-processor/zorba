(: fn:count evaluates the return clause although the type of the clause gives one item per tuple. :)
import module namespace ref = "http://zorba.io/modules/reference";
let $n := <a/>
return (count(for $i in (1, 2) return ref:reference($n)), ref:has-reference($n))
