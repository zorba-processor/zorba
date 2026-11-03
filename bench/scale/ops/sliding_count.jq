(: Sliding window by count: the average of v over 10 records, from each record. :)
sum(for sliding window $w in @INPUT@
      start at $s when true
      only end at $e when $e - $s eq 9
    return avg(local:num($w.v)))
