jsoniq version "1.0";
(: JSONiq 5.1.5: a key that appears twice in a merging constructor is an error. :)
let $a := [ {"k":"a"}, {"k":"a"} ]
return [
  for tumbling window $w in $a[]
    start when true
    end when false
  return {| for $x in $w return { $x.k : 1 } |}
]
