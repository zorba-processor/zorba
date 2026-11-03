jsoniq version "1.0";
(: A condition on the variable of an outer for must be evaluated per outer tuple. :)
[
  for $r in ({"t":1}, {"t":2})
  for tumbling window $w in (1, 2, 3)
    start $s when $s gt $r.t
  return [ $r.t, $w ]
]
