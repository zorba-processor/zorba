jsoniq version "1.0";
(: Object and array constructors in the return clause, [ $w ] among them. :)
let $a := [ {"ts":"a","v":1}, {"ts":"b","v":2} ]
return [
  for tumbling window $w in $a[]
    start when true
    end when false
  return {
    "first": $w[1].ts,
    "last": $w[last()].ts,
    "vals": [ $w.v ],
    "n": count($w),
    "nested": { "sum": sum($w.v), "objs": [ $w ] }
  }
]
