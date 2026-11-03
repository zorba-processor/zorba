jsoniq version "1.0";
(: Positions count within each binding sequence, so they restart per parent. :)
let $docs := ({"sensor":"a","r":[1,2,3]}, {"sensor":"b","r":[7]}, {"sensor":"c","r":[9,8]})
return [
  for $d in $docs
  for sliding window $w in $d.r[]
    start at $s when true
    only end at $e when $e - $s eq 1
  return { "s": $d.sensor, "from": $s, "w": [ $w ] }
]
