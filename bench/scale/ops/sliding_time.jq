(: Sliding window by time: the average of v over 1 minute from each record. :)
sum(for sliding window $w in @INPUT@
      start $s when true
      only end next $n when empty($n) or local:t($n) - local:t($s) gt xs:dayTimeDuration("PT1M")
    return avg(local:num($w.v)))
