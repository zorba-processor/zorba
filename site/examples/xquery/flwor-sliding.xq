let $temps := (21, 23, 22, 26, 28, 27)
for sliding window $w in $temps
    start at $s when true()
    only end at $e when $e - $s eq 2
return round-half-to-even(avg($w), 1)
