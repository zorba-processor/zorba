jsoniq version "1.0";
(: A JSON null is an item: only the first item has no previous item. :)
let $a := [null, 1, null]
return [
  for tumbling window $w in $a[]
    start $s previous $p next $n when true
  return { "s": $s, "noprev": empty($p), "p": $p, "nonext": empty($n) }
]
