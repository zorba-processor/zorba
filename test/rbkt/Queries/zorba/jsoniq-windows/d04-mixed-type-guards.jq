jsoniq version "1.0";
(: XQuery 3.0 2.3.4: a branch that is not selected raises no error, so a guard
   makes a comparison on a field with mixed types safe. :)
let $a := [ {"i":1,"v":1}, {"i":2,"v":"2"}, {"i":3,"v":true}, {"i":4,"v":3.5}, {"i":5,"v":null},
            {"i":6,"v":5}, {"i":7,"v":[1]}, {"i":8,"v":{"a":1}}, {"i":9} ]
return [
  [ for tumbling window $w in $a[]
      start $s when if ($s.v instance of xs:decimal) then $s.v gt 2 else false
    return [ $w.i ] ],
  [ for tumbling window $w in $a[]
      start $s when typeswitch ($s.v) case $n as xs:decimal return $n gt 2 default return false
    return [ $w.i ] ]
]
