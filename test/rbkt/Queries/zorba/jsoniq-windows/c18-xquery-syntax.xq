(: The same data through the XQuery syntax: jn:members and the call syntax. :)
let $a := jn:parse-json('[ {"v":1}, {"v":2}, {"v":3} ]')
return [
  for tumbling window $w in jn:members($a)
    start at $i when fn:true()
    end at $j when $j - $i eq 1
  return fn:sum(for $x in $w return $x("v"))
]
