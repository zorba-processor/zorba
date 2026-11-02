jsoniq version "1.0";
(: group by per sensor, order each group by time, window within the group.
   Records without a sensor are removed first. :)
let $data := [ {"sensor":"b","t":2,"v":20}, {"sensor":"a","t":1,"v":1}, {"sensor":"a","t":3,"v":3},
               {"sensor":"b","t":1,"v":10}, {"sensor":"a","t":2,"v":2}, {"v":99} ]
return [
  for $r in $data[]
  where exists($r.sensor)
  group by $s := $r.sensor
  order by $s
  return {
    "sensor": $s,
    "avgs": [
      for sliding window $w in (for $x in $r order by $x.t return $x)
        start at $i when true
        only end at $j when $j - $i eq 1
      return avg($w.v)
    ]
  }
]
