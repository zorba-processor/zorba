(: Function items found by name that read the context position and the
   context size, each called twice. :)
(10, 20, 30) !
  (let $p := function-lookup(xs:QName("fn:position"), 0)
   let $l := function-lookup(xs:QName("fn:last"), 0)
   return ($p(), $p(), $l(), $l()))
