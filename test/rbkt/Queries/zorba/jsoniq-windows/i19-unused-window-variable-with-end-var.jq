jsoniq version "1.0";
(: The window variable is not read, the variables of a start or end are: their items stay available. :)
let $a := [1, 2, 3, 4]
return [
  for sliding window $w in $a[]
    start $s when true
    only end $e when $s ne 1 and $e eq $s + 1
  return [ $s, $e ],
  for sliding window $w in $a[]
    start when true
    only end $e when $e eq 2 or $e eq 4
  return $e
]
