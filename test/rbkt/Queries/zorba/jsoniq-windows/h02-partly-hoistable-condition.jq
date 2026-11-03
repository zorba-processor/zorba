jsoniq version "1.0";
(: In the condition, $refs[] does not depend on the window, $r.k eq $s.v does. :)
let $refs := [ {"k":1}, {"k":3} ]
let $a := [ {"v":1}, {"v":2}, {"v":3} ]
return [
  for tumbling window $w in $a[]
    start $s when some $r in $refs[] satisfies $r.k eq $s.v
  return [ $w.v ]
]
