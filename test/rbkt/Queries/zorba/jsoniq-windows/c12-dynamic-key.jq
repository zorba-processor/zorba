jsoniq version "1.0";
(: JSONiq 5.5.1: a variable, a parenthesized expression or a string as the key. :)
let $key := "v"
let $a := [ {"v":1}, {"v":2}, {"v":3} ]
return [
  for tumbling window $w in $a[]
    start $s when $s.$key gt 1
  return [ $s.$key, $s.("v"), $s."v" ]
]
