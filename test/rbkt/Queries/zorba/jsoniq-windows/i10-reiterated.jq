jsoniq version "1.0";
(: The clause runs again for each outer tuple, and a return clause is reevaluated after an early stop. :)
[
  [
    for $k in 1 to 2
    for sliding window $w in 1 to 2 + $k
      start $s when true
      only end $e when $e - $s eq 1
    return [ $k, [ $w ] ]
  ],
  [
    for $o in 1 to 2
    return [ (
      for sliding window $w in 1 to 5 + $o
        start $s when true
        only end $e when $e - $s eq 1
      return [ $w ])[1] ]
  ]
]
