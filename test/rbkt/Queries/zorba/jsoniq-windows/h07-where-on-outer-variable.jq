jsoniq version "1.0";
(: A where clause on an outer variable, which rewrites can move before the window. :)
let $rs := ({"k":"a","xs":[1]}, {"k":"b","xs":[2,3]})
return [
  for $r in $rs
  for tumbling window $w in $r.xs[]
    start at $i when true
  where $r.k eq "b"
  return [ $w ]
]
