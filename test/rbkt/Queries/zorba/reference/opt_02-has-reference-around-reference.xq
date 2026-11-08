(: Two calls of ref:has-reference with the same argument are not the same call: ref:reference runs between them. :)
import module namespace ref = "http://zorba.io/modules/reference";
declare variable $n := <a/>;
(ref:has-reference($n), string-length(ref:reference($n)) gt 0, ref:has-reference($n))
