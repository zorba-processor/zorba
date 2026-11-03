jsoniq version "1.0";
(: The end condition reads the item after the current one, and no start variable. :)
let $a := [1, 2, 3]
return [
  for sliding window $w in $a[]
    start when true
    only end next $n when empty($n)
  return [ $w ]
]
