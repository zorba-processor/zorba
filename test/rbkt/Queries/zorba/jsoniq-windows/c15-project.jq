jsoniq version "1.0";
let $a := [ {"v":1,"x":2}, {"v":3,"y":4} ]
return [
  for tumbling window $w in $a[]
    start when true
    end when false
  return [ project($w, "v") ]
]
