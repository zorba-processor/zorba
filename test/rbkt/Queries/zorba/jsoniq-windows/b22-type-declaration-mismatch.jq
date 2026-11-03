jsoniq version "1.0";
(: XQuery 3.0 2.5.5: a value that does not match the declared type is a type error. :)
let $a := [ 1, "a" ]
return [
  for tumbling window $w as integer+ in $a[]
    start when true
    end when false
  return count($w)
]
