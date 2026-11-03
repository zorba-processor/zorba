jsoniq version "1.0";
[
  for $win at $i in (
    for tumbling window $w in (1, 2, 3)
      start at $s when true
      end at $e when $e - $s eq 1
    return [ $w ])
  return { "i": $i, "size": size($win) }
]
