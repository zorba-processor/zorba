jsoniq version "1.0";
(: JSONiq extension 6.1: the effective boolean value of an object is true. :)
let $a := [ {"v":1}, {"v":2}, {"v":3} ]
return [
  for tumbling window $w in $a[]
    start $s when ($s)[$$.v eq 2]
  return [ $w.v ]
]
