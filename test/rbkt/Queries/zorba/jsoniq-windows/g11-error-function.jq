jsoniq version "1.0";
(: F&O 3.0 3.1.1: fn:error from a condition. :)
let $a := [1]
return [
  for sliding window $w in $a[]
    start $s when $s eq 1 and error(xs:QName("local:boom"))
    end when true
  return 1
]
