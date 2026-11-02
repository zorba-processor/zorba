jsoniq version "1.0";
(: JSONiq navigation inside the conditions. :)
let $a := [ {"ts":"t1","tags":["x","y"],"o":{"p":{"q":1}}},
            {"ts":"t2","tags":[],"o":{}},
            {"ts":"t3","tags":["y"]} ]
return [
  [ for tumbling window $w in $a[] start $s when some $t in $s.tags[] satisfies $t eq "y" return [ $w.ts ] ],
  [ for tumbling window $w in $a[] start $s when $s.tags[[1]] eq "x" return [ $w.ts ] ],
  [ for tumbling window $w in $a[] start $s when exists($s.tags[][$$ eq "y"]) return [ $w.ts ] ],
  [ for tumbling window $w in $a[] start $s when size($s.tags) gt 0 return [ $w.ts ] ],
  [ for tumbling window $w in $a[] start $s when $s.o.p.q eq 1 end $e when $e.o.p.q eq 2 return [ $w.ts ] ]
]
