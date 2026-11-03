jsoniq version "1.0";
(: JSONiq extension 6.16, 6.17 and JSONiq 5.2.3: null equals only null and is
   smaller than any other atomic; a comparison with a missing field is empty, hence false. :)
let $r := [ {"i":1,"v":10}, {"i":2}, {"i":3,"v":null}, {"i":4,"v":3} ]
return [
  [ for tumbling window $w in $r[] start $s when $s.v lt 5 return $s.i ],
  [ for tumbling window $w in $r[] start $s when $s.v gt 5 return $s.i ],
  [ for tumbling window $w in $r[] start $s when $s.v eq null return $s.i ],
  [ for tumbling window $w in $r[] start $s when $s.v ne null return $s.i ],
  [ for tumbling window $w in $r[] start $s when $s.v le null return $s.i ],
  [ for tumbling window $w in $r[] start $s when $s.v ge null return $s.i ],
  [ for tumbling window $w in $r[] start $s when $s.v = null return $s.i ],
  [ for tumbling window $w in $r[] start $s when $s.v != 5 return $s.i ],
  [ for tumbling window $w in $r[] start $s when $s.v < 5 return $s.i ],
  [ for tumbling window $w in $r[] start $s when empty($s.v) return $s.i ],
  [ for tumbling window $w in $r[] start $s when $s.v instance of null return $s.i ]
]
