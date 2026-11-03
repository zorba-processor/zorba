jsoniq version "1.0";
(: Positions below 1 select nothing: the item before the window is not part of it.
   The window of the second item is (2), so $w[last() - 1] is at position 0. :)
let $a := [1, 2, 3]
return [
  for tumbling window $w in $a[]
    start when true
    end when true
  let $zero := count($w) - 1
  return [ [ $w[0] ], [ $w[last() - 1] ], [ $w[$zero] ], [ $w[-1] ] ]
]
