jsoniq version "1.0";
(: The end condition reads a start variable through a function item, a nested FLWOR, a user function and a let. :)
declare function local:plus-one($x) { $x + 1 };
let $a := [1, 2, 3]
return [
  [ for sliding window $w in $a[]
      start $s when true
      only end $e when (function() { $s + 1 })() eq $e
    return [ $w ] ],
  [ for sliding window $w in $a[]
      start $s when true
      only end $e when $e eq (for $x in $s return $x + 1)
    return [ $w ] ],
  [ for sliding window $w in $a[]
      start $s when true
      only end $e when local:plus-one($s) eq $e
    return [ $w ] ],
  [ for sliding window $w in $a[]
      start at $p when true
      only end at $q when (let $d := $q - $p return $d eq 1)
    return [ $w ] ],
  [ for sliding window $w in $a[]
      start previous $p when true
      only end $e when $e eq (if (empty($p)) then 2 else $p + 2)
    return [ $w ] ],
  [ for sliding window $w in $a[]
      start next $n when true
      only end $e when $e eq $n
    return [ $w ] ]
]
