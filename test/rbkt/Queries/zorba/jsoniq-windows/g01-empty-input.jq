jsoniq version "1.0";
(: XQuery 3.0 3.10.4.3: no windows, so no tuples. :)
let $e := []
return [
  [ for tumbling window $w in () start when true return 1 ],
  [ for tumbling window $w in $e[] start when true return 1 ],
  [ for tumbling window $w in $e[] start when true end when true return 1 ],
  [ for tumbling window $w in $e[] start when true only end when true return 1 ],
  [ for sliding window $w in $e[] start when true end when true return 1 ],
  [ for sliding window $w in $e[] start when true only end when true return 1 ]
]
