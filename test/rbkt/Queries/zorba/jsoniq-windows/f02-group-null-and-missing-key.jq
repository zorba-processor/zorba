jsoniq version "1.0";
(: XQuery 3.0 3.10.7: an empty grouping key forms its own group; null is a value. :)
let $data := [ {"i":1,"k":"x"}, {"i":2,"k":null}, {"i":3}, {"i":4,"k":"x"}, {"i":5,"k":null} ]
return [
  for $r in $data[]
  group by $k := $r.k
  order by if (empty($k)) then 0 else if ($k instance of null) then 1 else 2
  return {
    "isnull": $k instance of null,
    "pairs": [ for tumbling window $w in $r start at $i when true end at $j when $j - $i eq 1 return [ $w.i ] ]
  }
]
