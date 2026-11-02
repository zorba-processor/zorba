jsoniq version "1.0";
(: Input: a binding sequence with atomics, null, objects and arrays. :)
let $a := [ 1, "a", null, {"k":1}, [2], true ]
return [
  for tumbling window $w in $a[]
    start at $s when true
    end at $e when $e - $s eq 1
  return [ $w ]
]
