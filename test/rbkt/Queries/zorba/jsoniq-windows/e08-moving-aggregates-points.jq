jsoniq version "1.0";
(: Moving average, min, max and sum over 3 points. :)
let $a := [ {"v":10}, {"v":20}, {"v":30}, {"v":40}, {"v":50} ]
return [
  for sliding window $w in $a[]
    start at $s when true
    only end at $e when $e - $s eq 2
  return { "avg": avg($w.v), "min": min($w.v), "max": max($w.v), "sum": sum($w.v) }
]
