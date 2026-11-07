(: XQUF 3.0: the pending update list of the modify clause must not contain a
   upd:put primitive. :)
copy $c := <x/>
modify fn:put($c, "http://zorba.io/tests/transform-targets/put.xml")
return $c
