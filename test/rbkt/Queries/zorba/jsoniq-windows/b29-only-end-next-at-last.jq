jsoniq version "1.0";
(: `next` is empty only at the last item. :)
let $a := [1, 2, 3]
return [
  for tumbling window $w in $a[]
    start when true
    only end next $n when empty($n)
  return [ $w ]
]
