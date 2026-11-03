jsoniq version "1.0";
(: The window variable may have a type declaration. :)
let $a := [ {"a":1}, {"a":2} ]
return [
  for tumbling window $w as object+ in $a[]
    start when true
  return count($w)
]
