jsoniq version "1.0";
(: JSONiq 5.5.3: $$ is the context item of a predicate. :)
let $a := [ {"ts":"t1","v":1}, {"ts":"t2","v":2}, {"ts":"t3","v":3} ]
return [
  for tumbling window $w in $a[]
    start when true
    end when false
  return [ [ $w[$$.v gt 1].ts ], [ $w ! ($$.v * 2) ], $w[2].v, [ $w[position() gt 2].ts ] ]
]
