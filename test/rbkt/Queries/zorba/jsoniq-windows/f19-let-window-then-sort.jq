jsoniq version "1.0";
(: A let over the window read after the tuples are sorted. :)
[
  for tumbling window $w in (1, 2, 3, 4)
    start at $s when true
    end at $e when $e - $s eq 1
  let $l := $w
  order by $l[1] descending
  return [ $l ]
]
