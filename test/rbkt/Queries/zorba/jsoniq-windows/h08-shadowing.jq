jsoniq version "1.0";
(: Window variables shadow outer variables of the same name. :)
[
  let $s := 5
  for tumbling window $w in (1, 6)
    start $s when $s gt 1
  return $s,
  for $w in (1, 2)
  for tumbling window $w in (8, 9)
    start when true
  return [ $w ]
]
