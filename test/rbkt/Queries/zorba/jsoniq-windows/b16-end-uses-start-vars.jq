jsoniq version "1.0";
(: XQuery 3.0 3.10.4: start variables are in scope in the end condition.
   From 1: $sp is empty, no end item. From 5: $sp is 1, the end item is 2.
   From 2: looks for 6. From 9: looks for 3. Only one window is generated. :)
let $a := [1, 5, 2, 9]
return [
  for sliding window $w in $a[]
    start $s previous $sp when true
    only end $e when $e eq $sp + 1
  return [ $w ]
]
