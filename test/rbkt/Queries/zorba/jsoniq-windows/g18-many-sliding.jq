jsoniq version "1.0";
(: 4998 windows of 3 items; their averages are 2 .. 4999. :)
let $ws := (
  for sliding window $w in (for $i in 1 to 5000 return {"i":$i})
    start at $s when true
    only end at $e when $e - $s eq 2
  return avg($w.i))
return [ count($ws), sum($ws) ]
