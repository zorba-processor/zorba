jsoniq version "1.0";
(: A window clause over the window variable of another one. :)
[
  for sliding window $a in 1 to 4
    start $s when true
    only end $e when $e - $s eq 2
  for sliding window $b in $a
    start $t when true
    only end $u when $u - $t eq 1
  return [ [ $a ], [ $b ] ]
]
