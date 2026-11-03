jsoniq version "1.0";
(: $$ of an enclosing map in the domain and in the condition of a window. :)
[
  (2, 3) ! [
    for tumbling window $w in (1 to $$)
      start at $s when true
      end at $e when $e - $s eq 1
    return sum($w)
  ],
  5 ! [ for tumbling window $w in (1, 6) start $s when $s gt $$ return $s ]
]
