jsoniq version "1.0";
(: A let on the parent used in the condition. A missing $lim makes the comparison
   empty, hence false, so `only end` gives no window for that parent. :)
let $docs := ({"lim":1,"xs":[1,2,3]}, {"lim":0,"xs":[4,5]}, {"xs":[6,7]})
return [
  for $d in $docs
  let $lim := $d.lim
  for sliding window $w in $d.xs[]
    start at $s when true
    only end at $e when $e - $s eq $lim
  return [ $w ]
]
