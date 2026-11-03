(: Tumbling window: the average of v per window of 1 minute from its first record. :)
sum(for tumbling window $w in @INPUT@
      start $s when true
      end next $n when empty($n) or local:t($n) - local:t($s) ge xs:dayTimeDuration("PT1M")
    return avg(local:num($w.v)))
