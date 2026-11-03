jsoniq version "1.0";
(: XQuery 3.0 3.10.4: positions are integers, the items keep their types.
   JSONiq names types without a prefix. :)
let $a := [ {"v":1}, {"v":2} ]
return [
  for tumbling window $w in $a[]
    start $s at $i previous $p when true
  return [ $s instance of object, $i instance of integer, $s.v instance of decimal, $p instance of object? ]
]
