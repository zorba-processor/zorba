jsoniq version "1.0";
(: Input: a nested array inside each object, one window clause per parent.
   $d.r[] is empty for an empty array and for a missing key (JSONiq: unboxing). :)
let $docs := ({"sensor":"a","r":[1,2,3,4,5]}, {"sensor":"b","r":[7]}, {"sensor":"c","r":[]}, {"sensor":"d"})
return [
  for $d in $docs
  for tumbling window $w in $d.r[]
    start at $s when true
    end at $e when $e - $s eq 1
  return { "s": $d.sensor, "w": [ $w ] }
]
