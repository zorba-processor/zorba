jsoniq version "1.0";
(: JSONiq 5.1.5: the merging constructor on the objects of a window. :)
let $a := [ {"k":"a","v":1}, {"k":"b","v":2}, {"k":"c","v":3} ]
return [
  for tumbling window $w in $a[]
    start at $i when true
    end at $j when $j - $i eq 1
  return {| for $x in $w return { $x.k : $x.v } |}
]
