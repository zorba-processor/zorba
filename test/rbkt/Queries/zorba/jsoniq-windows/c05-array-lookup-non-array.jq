jsoniq version "1.0";
(: JSONiq 5.5.2: array lookup on an item that is not an array is empty. :)
let $a := [ {"a":1}, {"a":2} ]
return [
  for tumbling window $w in $a[]
    start when true
    end when false
  return [ $w[[1]] ]
]
