jsoniq version "1.0";
(: A missing field contributes no item to $w.v, so count, sum, avg, min and max
   see only the present values. On no values, sum is 0, the others are empty (F&O 3.0 14.4). :)
let $a := [ {"v":10}, {}, {"v":20} ]
let $b := [ {}, {} ]
return [
  for $x in ($a, $b)
  for tumbling window $w in $x[]
    start when true
    end when false
  return { "n": count($w), "k": count($w.v), "sum": sum($w.v), "avg": avg($w.v), "min": min($w.v), "max": max($w.v) }
]
