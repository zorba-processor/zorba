jsoniq version "1.0";
[
  for tumbling window $w in (1, 2, 3, 4)
    start $s at $i when true
    end $e at $j when $j - $i eq 1
  where $e gt 2
  return $s
]
