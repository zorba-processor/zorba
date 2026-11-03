jsoniq version "1.0";
let $a := [1, 2, 3, 4, 5]
return [
  for tumbling window $w in $a[]
    start at $s when true
    end at $e when $e - $s eq 1
  return [
    for sliding window $v in $w
      start at $i when true
      only end at $j when $j - $i eq 1
    return sum($v)
  ]
]
