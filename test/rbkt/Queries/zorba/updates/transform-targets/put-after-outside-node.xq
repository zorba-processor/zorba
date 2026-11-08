(: A node outside the copy is renamed next to a fn:put: XUDY0014 is raised, not
   XUDY0037. :)
let $outside := <o/>
return
  copy $c := <x/>
  modify (fn:put($c, "http://zorba.io/tests/transform-targets/put.xml"),
          rename node $outside as "z")
  return $c
