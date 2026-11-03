jsoniq version "1.0";
let $all := (
  for tumbling window $w in (1, 2, 3, 4, 5)
    start at $s when true
    only end at $e when $e - $s eq 1
  return $w)
return [ count($all), sum($all), $all[4] ]
