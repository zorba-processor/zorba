jsoniq version "1.0";
(: Tumbling windows over the output of a sliding window. :)
let $a := [ {"v":1}, {"v":3}, {"v":5}, {"v":7} ]
return [
  for tumbling window $o in (
      for sliding window $i in $a[]
        start at $s when true
        only end at $e when $e - $s eq 1
      return { "avg": avg($i.v) })
    start at $s when true
    end at $e when $e - $s eq 1
  return [ $o.avg ]
]
