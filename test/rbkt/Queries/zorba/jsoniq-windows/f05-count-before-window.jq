jsoniq version "1.0";
[
  for $x in (10, 20)
  count $c
  for tumbling window $w in (1, 2, 3)
    start at $s when true
    end at $e when $e - $s eq 1
  return [ $c, $x, $w ]
]
