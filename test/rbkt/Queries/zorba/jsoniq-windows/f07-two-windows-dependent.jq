jsoniq version "1.0";
(: A second window clause over the window of the first. :)
let $a := [1, 2, 3, 4, 5, 6]
return [
  for tumbling window $o in $a[]
    start at $s when true
    only end at $e when $e - $s eq 2
  for sliding window $i in $o
    start at $k when true
    only end at $l when $l - $k eq 1
  return [ $i ]
]
