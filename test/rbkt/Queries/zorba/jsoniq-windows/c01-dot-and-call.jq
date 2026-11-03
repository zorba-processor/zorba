jsoniq version "1.0";
(: JSONiq: $x.k and $x("k") select the same value, on variables and on $w[i]. :)
let $a := [ {"ts":"t1"}, {"ts":"t2"}, {"ts":"t3"} ]
return [
  for tumbling window $w in $a[]
    start $s at $i when true
    end at $j when $j - $i eq 1
  return [ $s.ts, $s("ts"), $w[1].ts, $w[1]("ts"), $w[last()].ts, $w[2].ts ]
]
