jsoniq version "1.0";
(: JSONiq extension 4.x: the call syntax iterates implicitly over the left-hand sequence. :)
let $a := [ {"ts":"t1"}, {"ts":"t2"}, {"ts":"t3"} ]
return [
  for tumbling window $w in $a[]
    start at $i when true
    end at $j when $j - $i eq 1
  return [ $w("ts") ]
]
