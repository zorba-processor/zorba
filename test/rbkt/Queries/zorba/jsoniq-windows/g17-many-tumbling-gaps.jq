jsoniq version "1.0";
(: Items between windows belong to no window. Windows (100,101), (200,201), ...;
   the one that starts at 10000 has no end item. Sum of 200k + 1 for k in 1..99. :)
let $ws := (
  for tumbling window $w in (1 to 10000)
    start $s at $p when $s mod 100 eq 0
    only end at $e when $e - $p eq 1
  return sum($w))
return [ count($ws), sum($ws) ]
