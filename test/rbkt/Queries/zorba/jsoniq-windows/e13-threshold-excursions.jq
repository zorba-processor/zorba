jsoniq version "1.0";
(: Periods above 50: the start crosses upward, the end is followed by a value
   below 50. The last excursion never ends (`only end`), so it is dropped. :)
let $a := [ {"v":40}, {"v":60}, {"v":70}, {"v":45}, {"v":55}, {"v":30}, {"v":80} ]
return [
  [ for tumbling window $w in $a[]
      start $s previous $p when $p.v lt 50 and $s.v ge 50
      only end next $n when $n.v lt 50
    return [ $w.v ] ],
  [ for sliding window $w in $a[]
      start $s previous $p when $p.v lt 50 and $s.v ge 50
      only end next $n when $n.v lt 50
    return [ $w.v ] ]
]
