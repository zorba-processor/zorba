jsoniq version "1.0";
(: JSONiq 5.7.3: null sorts first. XQuery 3.0 3.10.8: `empty least` and `empty greatest`
   place the empty sequence, here a missing field. :)
let $n := [ {"i":1,"t":3}, {"i":2,"t":null}, {"i":3,"t":1} ]
let $m := [ {"i":1,"t":3}, {"i":2}, {"i":3,"t":1} ]
return [
  [ for sliding window $w in (for $x in $n[] order by $x.t return $x)
      start at $s when true only end at $e when $e - $s eq 1 return [ $w.i ] ],
  [ for sliding window $w in (for $x in $n[] order by $x.t empty greatest return $x)
      start at $s when true only end at $e when $e - $s eq 1 return [ $w.i ] ],
  [ for sliding window $w in (for $x in $m[] order by $x.t empty least return $x)
      start at $s when true only end at $e when $e - $s eq 1 return [ $w.i ] ],
  [ for sliding window $w in (for $x in $m[] order by $x.t empty greatest return $x)
      start at $s when true only end at $e when $e - $s eq 1 return [ $w.i ] ]
]
