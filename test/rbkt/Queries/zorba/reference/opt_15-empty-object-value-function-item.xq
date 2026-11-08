(: An object constructor evaluates a value of the empty sequence type that calls a function item. :)
import module namespace ref = "http://zorba.io/modules/reference";
declare namespace an = "http://zorba.io/annotations";

let $n := <a/>
let $f := %an:nondeterministic function($m as node()) as empty-sequence()
           { if (string-length(ref:reference($m)) gt 0) then () else () }
return ({ "a" : $f($n) }("a") instance of js:null, ref:has-reference($n))
