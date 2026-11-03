jsoniq version "1.0";
(: [] on a field that is an array in some records and absent or scalar in others. :)
let $a := [ {"list":[1,2]}, {"list":"x"}, {}, {"list":[3]} ]
return [
  for tumbling window $w in $a[]
    start when true
    end when false
  return sum($w.list[])
]
