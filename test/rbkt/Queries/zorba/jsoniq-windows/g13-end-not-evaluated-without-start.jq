jsoniq version "1.0";
(: XQuery 3.0 3.10.4: the end condition is looked for only after a start item. :)
let $a := [1, 2]
return [
  [ for tumbling window $w in $a[] start $s when $s lt 0 end when error(xs:QName("local:boom")) return 1 ],
  [ for sliding window $w in $a[] start $s when $s lt 0 end when error(xs:QName("local:boom")) return 1 ]
]
