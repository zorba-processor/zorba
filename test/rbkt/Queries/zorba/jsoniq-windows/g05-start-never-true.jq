jsoniq version "1.0";
let $a := [1, 2, 3]
return [
  [ for tumbling window $w in $a[] start when false return 1 ],
  [ for tumbling window $w in $a[] start when false end when true return 1 ],
  [ for sliding window $w in $a[] start when false end when true return 1 ],
  [ for sliding window $w in $a[] start when false only end when true return 1 ]
]
