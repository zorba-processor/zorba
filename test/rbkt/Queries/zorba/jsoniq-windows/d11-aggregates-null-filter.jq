jsoniq version "1.0";
let $a := [ {"v":1}, {"v":null}, {"v":3} ]
return [
  for tumbling window $w in $a[]
    start when true
    end when false
  let $n := $w.v[$$ instance of xs:decimal]
  return { "sum": sum($n), "avg": avg($n), "min": min($n), "max": max($n) }
]
