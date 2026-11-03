jsoniq version "1.0";
(: An end condition on the current item alone closes all the open windows at once, in order. :)
let $a := [1, 2, 3, 4, 5]
return [
  [ for sliding window $w in $a[]
      start $s when $s le 3
      only end $e when $e eq 4
    return [ $s, $e, [ $w ] ] ],
  [ for sliding window $w in $a[]
      start $s when true
      only end $e when $e mod 2 eq 0
    return [ $s, $e, [ $w ] ] ],
  [ for sliding window $w in $a[]
      start $s when $s ge 4
      end $e when $e eq 99
    return [ $s, $e, [ $w ] ] ]
]
