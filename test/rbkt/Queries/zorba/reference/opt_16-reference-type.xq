(: The result of ref:reference is an xs:anyURI, as documented, at every optimization level. :)
import module namespace ref = "http://zorba.io/modules/reference";
let $n := <a/>
let $r := ref:reference($n)
return (
  $r instance of xs:anyURI, $r instance of xs:string,
  ref:reference($n) instance of xs:anyURI, ref:reference($n) instance of xs:string,
  ref:dereference($r) is $n
)
