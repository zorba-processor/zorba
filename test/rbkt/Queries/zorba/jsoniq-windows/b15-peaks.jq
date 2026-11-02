jsoniq version "1.0";
(: XQuery 3.0 3.10.4: a start item larger than both neighbors. At the first and
   last item a comparison with the empty sequence is empty, hence false. :)
let $a := [1, 3, 2, 5, 4]
return [
  for tumbling window $w in $a[]
    start $s previous $p next $n when $s gt $p and $s gt $n
  return [ $w ]
]
