(: The domain of a window clause. :)
declare variable $g := 1 to 6;

for tumbling window $w in $g
    start at $s when fn:true()
    end at $e when $e - $s eq 1
return fn:sum($w)
